#ifndef HEADER_UTIL_CURSOR
#define HEADER_UTIL_CURSOR

#include <vector>
#include <ranges>
#include <map>

#include "wz/Property.hpp"

#include "template/currenton.hpp"
#include "util/sprite_util.hpp"

#include "sdlms/cursor.hpp"

// 加载鼠标
class Cursor;

namespace util
{
    class CursorUtil final : public Currenton<CursorUtil>
    {
    public:
        CursorUtil();
        std::unordered_map<Cursor::Event, DynamicSprite> load();

    private:
        SpriteUtil *_sprite_util;

    private:
        wz::Node *_ui_node;
    };
}

#endif