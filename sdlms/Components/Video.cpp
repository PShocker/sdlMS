#include "Video.h"
#include "Core/Window.h"
#include <thread>

Video *Video::load(const std::string &url, int width, int height)
{
    if (!inited)
    {
        // 初始化FFmpeg
        avformat_network_init();
        inited = true;
    }
    // 打开视频文件
    AVFormatContext *formatContext = avformat_alloc_context();
    if (avformat_open_input(&formatContext, url.c_str(), NULL, NULL) != 0)
    {
        return nullptr;
    }
    if (avformat_find_stream_info(formatContext, NULL) < 0)
    {
        return nullptr;
    }
    const AVCodec *codec;
    int videoStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (videoStreamIndex == -1 || !codec)
    {
        return nullptr;
    }
    AVCodecParameters *codecParameters = formatContext->streams[videoStreamIndex]->codecpar;
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (!codecContext)
    {
        return nullptr;
    }
    if (avcodec_parameters_to_context(codecContext, codecParameters) < 0)
    {
        return nullptr;
    }
    // 设置解码线程
    codecContext->thread_count = std::thread::hardware_concurrency();
    
    if (avcodec_open2(codecContext, codec, nullptr) < 0)
    {
        return nullptr;
    }

    struct SwsContext *swsContext = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
                                                   codecContext->width, codecContext->height, AV_PIX_FMT_YUV420P,
                                                   SWS_BICUBIC, NULL, NULL, NULL);

    return new Video(formatContext, codecContext, videoStreamIndex, swsContext, width, height);
}

Video::Video(AVFormatContext *formatContext, AVCodecContext *codecContext, int videoStreamIndex, SwsContext *swsContext, int width, int height) : formatContext(formatContext),
                                                                                                                                                  codecContext(codecContext),
                                                                                                                                                  videoStreamIndex(videoStreamIndex),
                                                                                                                                                  swsContext(swsContext),
                                                                                                                                                  width(width),
                                                                                                                                                  height(height)

{
    texture = SDL_CreateTexture(Window::get_renderer(), SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING,
                                codecContext->width, codecContext->height);
}
