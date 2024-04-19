#ifndef HEADER_UTIL_FREETYPE
#define HEADER_UTIL_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>
#include <cstdint>
#include <string>
#include <SDL2/SDL.h>

#include "template/currenton.hpp"
#include "sdlms/sprite.hpp"

// 加载字体
namespace util
{
    class FreeTypeUtil final : public Currenton<FreeTypeUtil>
    {
    public:
        FreeTypeUtil();
        Sprite load_str(const std::u16string &s, int fontSize = 16, int b0 = 255, int g0 = 255, int r0 = 255,
                        bool back = false, int b1 = 0, int g1 = 0, int r1 = 0, int a1 = 255);

    public:
        const std::string filename_prefix = "../font";
        FT_Library *_library;
        FT_Face *_face;

    private:
        SDL_Renderer *_renderer;
    };
}

#endif