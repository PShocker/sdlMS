#include <SDL2/SDL.h>
#include <stdio.h>
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}
#include "sdlms/main.hpp"

static Main g_main;

int main(int argc, char *argv[])
{
    // 打开音频文件
    AVFormatContext *format_ctx = nullptr;
    if (avformat_open_input(&format_ctx, argv[1], nullptr, nullptr) != 0)
    {
        // std::cerr << "Failed to open audio file" << std::endl;
        return -1;
    }
    return g_main.run(argc, argv);
}