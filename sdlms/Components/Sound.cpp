module;

#include <SDL3/SDL.h>
#include "wz/Property.hpp"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

module components;

static std::unordered_map<wz::Node *, SoundWarp *> cache;

// 混合两个音频信号
void mixAudio(uint8_t *audio1, uint8_t *audio2, uint8_t *output, int length)
{
    for (int i = 0; i < length; i++)
    {
        uint16_t mixedSample = (uint16_t)audio1[i] + (uint16_t)audio2[i];
        // 限制输出范围
        if (mixedSample > 65535)
            mixedSample = 65535;
        output[i] = (uint8_t)mixedSample;
    }
}

static void SDLCALL FeedTheAudioStreamMore(void *userdata, SDL_AudioStream *astream, int additional_amount, int total_amount)
{
    if (additional_amount > 0)
    {
        Uint8 *data = SDL_stack_alloc(Uint8, additional_amount);
        SDL_memset(data, 0, additional_amount);

        for (int i = 0; i < Sound::sound_list.size(); ++i)
        {
            auto souw = Sound::sound_list[i];
            if (souw != nullptr)
            {
                auto amount = std::min((unsigned long long)additional_amount, souw->pcm_data.size() - souw->offset);
                mixAudio(data, souw->pcm_data.data() + souw->offset, data, amount);
                souw->offset += amount;
                if (souw->offset >= souw->pcm_data.size())
                {
                    if (souw->circulate)
                    {
                        souw->offset = 0;
                    }
                    else
                    {
                        Sound::sound_list[i] = nullptr;
                    }
                }
            }
        }
        SDL_PutAudioStreamData(astream, data, additional_amount);
        SDL_stack_free(data);
    }

    // Uint8 *data = SDL_stack_alloc(Uint8, additional_amount);
    // SDL_MixAudio(data, sou->pcm_data.data(), SDL_AUDIO_S16, additional_amount, 1);
    // SDL_PutAudioStreamData(astream, sou->pcm_data.data(), total_amount * sizeof(float));
    // SDL_stack_free(data);
    // SDL_MixAudio(astream, sou->pcm_data.data(), additional_amount, SDL_AUDIO_S16, 1);
}

bool Sound::init()
{
    SDL_AudioSpec spec;
    spec.channels = 2;
    spec.format = SDL_AUDIO_S16;
    spec.freq = 22050;
    auto stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, FeedTheAudioStreamMore, NULL);
    if (!stream)
    {
        return false;
    }
    SDL_ResumeAudioStreamDevice(stream);
    return true;
}

SoundWarp::SoundWarp(wz::Node *node)
{
    if (node->type == wz::Type::UOL)
    {
        node = dynamic_cast<wz::Property<wz::WzUOL> *>(node)->get_uol();
    }

    auto sound = dynamic_cast<wz::Property<wz::WzSound> *>(node);
    auto data = sound->get_raw_data();

    struct buffer_data
    {
        uint8_t *ptr;
        size_t size; ///< size left in the buffer
    } bd{data.data(), data.size()};

    auto avio_ctx_buffer = av_malloc(0x1000);
    AVIOContext *ioCtx = avio_alloc_context(
        (uint8_t *)avio_ctx_buffer, 0x1000,
        0, &bd,
        [](void *opaque, uint8_t *buf, int buf_size) -> int
        {
            struct buffer_data *bd = (struct buffer_data *)opaque;
            buf_size = FFMIN(buf_size, bd->size);

            if (!buf_size)
                return AVERROR_EOF;

            /* copy internal buffer data to buf */
            memcpy(buf, bd->ptr, buf_size);
            bd->ptr += buf_size;
            bd->size -= buf_size;

            return buf_size;
        },
        NULL, NULL);

    // 打开输入文件并读取音频流信息
    AVFormatContext *formatContext = avformat_alloc_context();
    formatContext->pb = ioCtx;

    if (avformat_open_input(&formatContext, nullptr, nullptr, nullptr) != 0)
    {
        // 处理打开文件失败的情况
        return;
    }
    if (avformat_find_stream_info(formatContext, nullptr) < 0)
    {
        // 处理找不到音频流信息的情况
        return;
    }

    const AVCodec *codec;
    int audioStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (audioStreamIndex == -1 || !codec)
    {
        // 打开音频解码器并分配解码上下文
        // 处理找不到音频流的情况
        return;
    }
    AVCodecParameters *codecParameters = formatContext->streams[audioStreamIndex]->codecpar;

    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (!codecContext)
    {
        // 处理无法分配解码上下文的情况
        return;
    }
    if (avcodec_parameters_to_context(codecContext, codecParameters) < 0)
    {
        // 处理无法设置解码器参数的情况
        return;
    }
    if (avcodec_open2(codecContext, codec, nullptr) < 0)
    {
        // 处理无法打开解码器的情况
        return;
    }

    // 解码并存储PCM数据
    std::vector<uint8_t> pcmData;
    // 解码音频帧
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    SwrContext *swrContext = swr_alloc();
    // 音频格式  输入的采样设置参数
    AVSampleFormat inFormat = codecContext->sample_fmt;
    // 出入的采样格式
    AVSampleFormat outFormat = AV_SAMPLE_FMT_S16;
    // 输入采样率
    int inSampleRate = codecContext->sample_rate;
    // 输出采样率
    int outSampleRate = inSampleRate;

    swr_alloc_set_opts2(&swrContext, &codecContext->ch_layout, outFormat, outSampleRate,
                        &codecContext->ch_layout, inFormat, inSampleRate, 0, NULL);

    if (swr_init(swrContext) < 0)
    {
        return;
    }

    int outChannelCount = codecContext->ch_layout.nb_channels;

    uint8_t *out_buffer = (uint8_t *)av_malloc(2 * outSampleRate);
    while (av_read_frame(formatContext, packet) >= 0)
    {
        if ((avcodec_send_packet(codecContext, packet)) >= 0)
        {
            auto error = avcodec_receive_frame(codecContext, frame);
            if (error == AVERROR(EAGAIN) || error == AVERROR_EOF || error < 0)
            {
                return;
            }
            else if (error == 0)
            {
                swr_convert(swrContext, &out_buffer, outSampleRate,
                            (const uint8_t **)frame->data,
                            frame->nb_samples);
                int size = av_samples_get_buffer_size(NULL, outChannelCount,
                                                      frame->nb_samples,
                                                      AV_SAMPLE_FMT_S16, 1);

                std::vector<uint8_t> out(out_buffer, out_buffer + size);

                pcmData.insert(pcmData.end(), out.begin(), out.end());

                av_frame_unref(frame);
            }
            av_packet_unref(packet);
        }
    }
    av_freep(&out_buffer);

    av_frame_free(&frame);
    av_packet_free(&packet);

    swr_free(&swrContext);

    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);

    pcm_data = pcmData;
}

SoundWarp *SoundWarp::load(wz::Node *node)
{
    if (cache.contains(node))
    {
        return cache[node];
    }
    else
    {
        SoundWarp *sou = new SoundWarp(node);
        cache[node] = sou;
        return sou;
    }
}

Sound::Sound(wz::Node *node)
{
    sou = SoundWarp::load(node);
}