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
        Sprite load_map_str(const std::u16string &s, int fontSize = 14);
        Sprite load_npc_str(const std::u16string &s, int fontSize = 16);

    public:
        const std::string filename_prefix = "C:/Windows/Fonts/";
        FT_Library *_library;
        FT_Face *_face;

    private:
        SDL_Renderer *_renderer;
    };
}

#endif