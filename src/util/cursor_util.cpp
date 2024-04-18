#include "util/cursor_util.hpp"
#include "util/wz_util.hpp"

class Cursor;

namespace util
{
    CursorUtil::CursorUtil()
    {
        _ui_node = WzUtil::current()->UI->get_root();

        _sprite_util = SpriteUtil::current();
    }

    EventSprite *CursorUtil::load()
    {
        auto node = _ui_node->find_from_path(u"Basic.img/Cursor");

        auto eventsprite = _sprite_util->load_event_sprite(Cursor::EventMap, node);

        return eventsprite;
    }
}