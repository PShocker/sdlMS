#ifdef __ANDROID__
#pragma once
#include <SDL2/SDL.h>
#include <vector>

class Android{
public:
    static unsigned char *get_file_buffer(const char *file);
    static unsigned int get_file_size(const char *file);
};
#endif
