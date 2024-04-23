#ifndef HEADER_UTIL_CURSOR
#define HEADER_UTIL_CURSOR

#include <vector>
#include <ranges>
#include <map>

#include "wz/Property.hpp"

#include "template/currenton.hpp"

#include "sdlms/cursor.hpp"

// 加载鼠标
class Cursor;

namespace util
{
    class CursorUtil final : public Currenton<CursorUtil>
    {
    public:
        CursorUtil();
        EventSprite load();

    private:
        wz::Node *_ui_node;
    };
}

#endif