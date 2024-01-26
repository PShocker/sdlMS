#ifndef HEADER_UTIL_FREETYPE
#define HEADER_UTIL_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>
#include <cstdint>
#include <string>
#include <SDL2/SDL.h>

#include "util/currenton.hpp"

// 加载声音
namespace util
{
    class FreeType final : public Currenton<FreeType>
    {
    public:
        FreeType();
        SDL_Texture *load_str(const std::wstring &s);

    public:
        const std::string filename_prefix = "C:/Users/Shocker/Desktop/sdlMS/data/";
        FT_Library *_library;
        FT_Face* _face;

    private:
        SDL_Renderer *_renderer;
    };
}

#endif