#include "util/cursor_util.hpp"
#include "util/wz_util.hpp"

class Cursor;

namespace util
{
    CursorUtil::CursorUtil()
    {
        _ui_node = WzUtil::current()->UI->get_root();
    }

    EventSprite CursorUtil::load()
    {
        auto node = _ui_node->find_from_path(u"Basic.img/Cursor");

        auto eventsprite = EventSprite::load_event_sprite(Cursor::EventMap, node);

        return eventsprite;
    }
}