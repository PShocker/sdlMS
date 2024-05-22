#include "File.h"

unsigned char *File::buffer(const char *path)
{
    SDL_RWops *rw = SDL_RWFromFile(path, "rb");
    if (rw == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open file: %s", SDL_GetError());
        return nullptr;
    }

    // 获取文件大小
    Sint64 file_size = SDL_RWsize(rw);
    if (file_size < 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't get file size: %s", SDL_GetError());
        SDL_RWclose(rw);
        return nullptr;
    }
    // 分配缓冲区来存储文件内容
    unsigned char *buffer = (unsigned char *)malloc(file_size);
    if (buffer == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Out of memory");
        SDL_RWclose(rw);
        return nullptr;
    }
    // 读取文件内容到缓冲区
    size_t read = SDL_RWread(rw, buffer, 1, file_size);
    if (read != file_size)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't read entire file: %s", SDL_GetError());
        free(buffer);
        SDL_RWclose(rw);
        return nullptr;
    }
    SDL_RWclose(rw);
    return buffer;
}

unsigned int File::size(const char *path)
{
    SDL_RWops *rw = SDL_RWFromFile(path, "rb");
    if (rw == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open file: %s", SDL_GetError());
        return 0;
    }

    // 获取文件大小
    auto size = SDL_RWsize(rw);
    SDL_RWclose(rw);
    return size;
}