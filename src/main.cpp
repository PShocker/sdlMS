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
    std::string s{avutil_configuration()};
    return g_main.run(argc, argv);
}