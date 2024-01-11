#include <SDL2/SDL.h>
#include <stdio.h>

#include "sdlms/main.hpp"

static Main g_main;

int main(int argc, char *argv[])
{
    return g_main.run(argc, argv);
}