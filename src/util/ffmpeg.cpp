#include "util/ffmpeg.hpp"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

namespace util
{
    std::vector<uint8_t> FFMPEG::decodeAudioToPCM(std::vector<uint8_t> data)
    {
        AVIOContext *ioCtx = avio_alloc_context(
            (unsigned char *)av_malloc(data.size()), data.size(),
            0, &data,
            [](void *opaque, uint8_t *buf, int buf_size) -> int
            {
                std::vector<uint8_t> *data = (std::vector<uint8_t> *)opaque;
                memcpy(buf, data->data(), buf_size);
                return buf_size;
            },
            NULL, NULL);

        // 打开输入文件并读取音频流信息
        AVFormatContext *formatContext = avformat_alloc_context();
        formatContext->pb = ioCtx;

        if (avformat_open_input(&formatContext, nullptr, nullptr, nullptr) != 0)
        {
            // 处理打开文件失败的情况
            // 返回空的std::vector作为错误处理示例，请根据实际情况进行修改
            return std::vector<uint8_t>();
        }
        if (avformat_find_stream_info(formatContext, nullptr) < 0)
        {
            // 处理找不到音频流信息的情况
            // 返回空的std::vector作为错误处理示例，请根据实际情况进行修改
            return std::vector<uint8_t>();
        }
        int audioStreamIndex = -1;
        AVCodecParameters *codecParameters = nullptr;
        for (unsigned int i = 0; i < formatContext->nb_streams; i++)
        {
            AVCodecParameters *codecpar = formatContext->streams[i]->codecpar;
            if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                audioStreamIndex = i;
                codecParameters = codecpar;
                break;
            }
        }
        if (audioStreamIndex == -1)
        {
            // 处理找不到音频流的情况
            // 返回空的std::vector作为错误处理示例，请根据实际情况进行修改
            return std::vector<uint8_t>();
        }

        // 打开音频解码器并分配解码上下文
        const AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
        if (!codec)
        {
            // 处理找不到解码器的情况
            // 返回空的std::vector作为错误处理示例，请根据实际情况进行修改
            return std::vector<uint8_t>();
        }
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if (!codecContext)
        {
            // 处理无法分配解码上下文的情况
            // 返回空的std::vector作为错误处理示例，请根据实际情况进行修改
            return std::vector<uint8_t>();
        }
        if (avcodec_parameters_to_context(codecContext, codecParameters) < 0)
        {
            // 处理无法设置解码器参数的情况
            // 返回空的std::vector作为错误处理示例，请根据实际情况进行修改
            return std::vector<uint8_t>();
        }
        if (avcodec_open2(codecContext, codec, nullptr) < 0)
        {
            // 处理无法打开解码器的情况
            // 返回空的std::vector作为错误处理示例，请根据实际情况进行修改
            return std::vector<uint8_t>();
        }

        // 分配音频数据缓冲区并读取解码后的PCM音频数据
        std::vector<uint8_t> pcmData;
        AVPacket packet;
        AVFrame *frame = av_frame_alloc();
        if (!frame)
        {
            // 处理无法分配帧结构的情况
            // 返回空的std::vector作为错误处理示例，请根据实际情况进行修改
            return std::vector<uint8_t>();
        }
        while (av_read_frame(formatContext, &packet) >= 0)
        {
            if (packet.stream_index == audioStreamIndex)
            {
                int ret = avcodec_send_packet(codecContext, &packet);
                while (ret >= 0)
                {
                    ret = avcodec_receive_frame(codecContext, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    {
                        break;
                    }
                    else if (ret < 0)
                    {
                        // 处理解码错误的情况
                        // 返回空的std::vector作为错误处理示例，请根据实际情况进行修改
                        return std::vector<uint8_t>();
                    }
                    // 解码成功，将PCM音频数据保存到std::vector
                    int dataSize = av_get_bytes_per_sample(codecContext->sample_fmt);
                    if (dataSize < 0)
                    {
                        // 处理样本格式错误的情况
                        // 返回空的std::vector作为错误处理示例，请根据实际情况进行修改
                        return std::vector<uint8_t>();
                    }
                    int numChannels = codecContext->ch_layout.nb_channels;
                    for (int i = 0; i < frame->nb_samples; i++)
                    {
                        for (int ch = 0; ch < numChannels; ch++)
                        {
                            pcmData.insert(pcmData.end(), frame->data[ch] + i * dataSize, frame->data[ch] + i * dataSize + dataSize);
                        }
                    }
                }
            }
            av_packet_unref(&packet);
        }
        av_frame_free(&frame);

        // 释放资源
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);

        return pcmData;
    }
}