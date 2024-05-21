#pragma once
#include <SDL2/SDL.h>

class File{
public:
    static unsigned char *buffer(const char *path);
    static unsigned int size(const char *path);
};