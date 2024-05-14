#pragma once
#include "Component.h"
#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

// 视频组件,可以是本地视频或者网络(直播)视频
class Video : public Component
{
public:
    static Video *load(const std::string &url, int width, int height);
    static inline bool inited = false;

public:
    const auto get_width() { return width; }
    const auto get_height() { return height; }

public:
    SDL_Texture *texture;
    int width;
    int height;

public:
    Video(AVFormatContext *formatContext, AVCodecContext *codecContext, int videoStreamIndex, SwsContext *swsContext, int width, int height);
    AVFormatContext *formatContext;
    AVCodecContext *codecContext;
    int videoStreamIndex;
    SwsContext *swsContext;
};