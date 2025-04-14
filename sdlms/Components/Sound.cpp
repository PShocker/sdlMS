#include "Sound.h"

#include "Core/Core.h"
#include "Resources/Wz.h"

#include <SDL3/SDL.h>
#include "wz/Property.hpp"
#include <thread>
#include <list>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

static SDL_Mutex *sound_list_mutex = SDL_CreateMutex();
static std::list<Sound> sound_list;
SDL_AudioStream *audio_stream;

// 混合两个音频信号
void mixAudio(Uint8 *audio1, Uint8 *audio2, Uint8 *output, int length)
{
    for (int i = 0; i < length; i++)
    {
        int16_t sample1 = reinterpret_cast<const int16_t *>(audio1)[i / 2];
        int16_t sample2 = reinterpret_cast<const int16_t *>(audio2)[i / 2];
        int32_t mixedSample = sample1 + sample2;
        // 限制范围
        if (mixedSample > INT16_MAX)
            mixedSample = INT16_MAX;
        if (mixedSample < INT16_MIN)
            mixedSample = INT16_MIN;
        // 存储混合结果
        reinterpret_cast<int16_t *>(output)[i / 2] = static_cast<int16_t>(mixedSample);
    }
}

static void SDLCALL FeedTheAudioStreamMore(void *userdata, SDL_AudioStream *astream, int additional_amount, int total_amount)
{
    if (additional_amount > 0)
    {
        SDL_LockMutex(sound_list_mutex);
        Uint8 *data = SDL_stack_alloc(Uint8, additional_amount);
        SDL_memset(data, 0, additional_amount * sizeof(Uint8));
        for (auto it = sound_list.begin(); it != sound_list.end();)
        {
            auto sou = &(*it);
            if (sou->delay > Window::dt_now)
            {
                ++it;
                continue;
            }
            auto souw = sou->souw;
            if (souw != nullptr)
            {
                auto amount = std::min((unsigned long long)additional_amount, (unsigned long long)souw->pcm_data.size() - (unsigned long long)sou->offset);
                mixAudio(data, souw->pcm_data.data() + sou->offset, data, amount);
                sou->offset += amount;
                if (sou->offset >= souw->pcm_data.size())
                {
                    if (sou->circulate)
                    {
                        sou->offset = 0;
                    }
                    else
                    {
                        it = sound_list.erase(it); // 删除当前元素并更新迭代器
                        continue;
                    }
                }
            }
            ++it;
        }
        SDL_PutAudioStreamData(astream, data, additional_amount);
        SDL_stack_free(data);
        SDL_UnlockMutex(sound_list_mutex);
    }
}

bool Sound::init()
{
    SDL_AudioSpec spec;
    spec.channels = 2;
    spec.format = SDL_AUDIO_S16;
    spec.freq = 44100;
    audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, FeedTheAudioStreamMore, NULL);
    if (!audio_stream)
    {
        return false;
    }
    SDL_ResumeAudioStreamDevice(audio_stream);
    return true;
}

Sound::Wrap *Sound::Wrap::load(wz::Node *node)
{
    static std::unordered_map<wz::Node *, Wrap *> cache;
    if (cache.contains(node))
    {
        return cache[node];
    }
    else
    {
        Wrap *wrap = new Wrap(node);
        cache[node] = wrap;
        return wrap;
    }
}

Sound::Wrap::Wrap(wz::Node *node)
{
    if (node->type == wz::Type::UOL)
    {
        node = dynamic_cast<wz::Property<wz::WzUOL> *>(node)->get_uol();
    }

    auto sound = dynamic_cast<wz::Property<wz::WzSound> *>(node);
    auto data = sound->get_raw_data();

    unsigned char *buffer = static_cast<unsigned char *>(av_malloc(data.size()));
    int buffer_size = data.size();

    memcpy(buffer, data.data(), data.size());
    AVIOContext *ioCtx = avio_alloc_context(
        (uint8_t *)buffer, // 输入数据
        buffer_size,       // 数据大小
        0,                 // 读写模式（0表示只读）
        NULL,              // 用户自定义数据
        NULL,              // 读取函数（将使用内存数据）
        NULL,              // 写入函数（未使用）
        NULL               // 清理函数（未使用）
    );

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
    // 设置多线程解码
    codecContext->thread_count = std::thread::hardware_concurrency();

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

    // 解码音频帧
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    SwrContext *swrContext = swr_alloc();
    // 音频格式  输入的采样设置参数
    AVSampleFormat Wraprmat = codecContext->sample_fmt;
    // 出入的采样格式
    AVSampleFormat outFormat = AV_SAMPLE_FMT_S16;
    // 输入采样率
    int inSampleRate = codecContext->sample_rate;
    // 输出采样率
    int outSampleRate = 44100;

    AVChannelLayout outChannel = {};
    outChannel.nb_channels = 2;

    int outChannelCount = outChannel.nb_channels;

    swr_alloc_set_opts2(&swrContext, &outChannel, outFormat, outSampleRate,
                        &codecContext->ch_layout, Wraprmat, inSampleRate, 0, NULL);

    if (swr_init(swrContext) < 0)
    {
        return;
    }

    // 分配输出缓冲区
    uint8_t *output_data = (uint8_t *)av_malloc(outSampleRate);
    while (av_read_frame(formatContext, packet) >= 0)
    {
        if ((avcodec_send_packet(codecContext, packet)) >= 0)
        {
            if (avcodec_receive_frame(codecContext, frame) == 0)
            {
                auto output_samples = swr_convert(swrContext, &output_data, outSampleRate,
                                                  (const uint8_t **)frame->data,
                                                  frame->nb_samples);

                pcm_data.insert(pcm_data.end(), output_data,
                                output_data + output_samples * 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16));
                av_frame_unref(frame);
            }
            av_packet_unref(packet);
        }
    }
    av_freep(&output_data);

    av_frame_free(&frame);
    av_packet_free(&packet);

    swr_free(&swrContext);

    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    av_freep(&ioCtx->buffer);
    avio_context_free(&ioCtx);
}

Sound::Sound(wz::Node *node, int d)
{
    souw = Sound::Wrap::load(node);
    delay = Window::dt_now + d;
}

Sound::Sound(const std::u16string &path, int d)
{
    auto node = Wz::Sound->get_root()->find_from_path(path);
    souw = Sound::Wrap::load(node);
    delay = Window::dt_now + d;
}

void Sound::push(Sound::Wrap *souw, int delay, int pos)
{
    SDL_LockMutex(sound_list_mutex);
    Sound sou;
    sou.delay = delay + Window::dt_now;
    sou.souw = souw;
    if (pos == -1)
    {
        sound_list.push_back(sou);
    }
    else
    {
        auto it = sound_list.begin();
        std::advance(it, pos); // 移动迭代器
        // 插入元素
        sound_list.insert(it, sou);
    }
    SDL_UnlockMutex(sound_list_mutex);
}

void Sound::push(Sound sou, int pos)
{
    SDL_LockMutex(sound_list_mutex);
    if (pos == -1)
    {
        sound_list.push_back(sou);
    }
    else
    {
        auto it = sound_list.begin();
        std::advance(it, pos); // 移动迭代器
        // 插入元素
        sound_list.insert(it, sou);
    }
    SDL_UnlockMutex(sound_list_mutex);
}

void Sound::remove(int pos)
{
    SDL_LockMutex(sound_list_mutex);
    if (pos == 0)
    {
        pos = 1;
    }
    auto it = sound_list.begin();
    std::advance(it, pos - 1); // 迭代器前进到第 pos 个元素
    sound_list.erase(it);      // 删除该元素
    SDL_UnlockMutex(sound_list_mutex);
}

Sound *Sound::at(int pos)
{
    SDL_LockMutex(sound_list_mutex);
    Sound *r = nullptr;
    if (pos < sound_list.size())
    {
        auto it = sound_list.begin();
        std::advance(it, pos); // 迭代器前进到第 pos 个元素
        r = &(*it);
    }
    SDL_UnlockMutex(sound_list_mutex);
    return r;
}