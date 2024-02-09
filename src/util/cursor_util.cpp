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

    std::unordered_map<Cursor::Event, DynamicSprite> CursorUtil::load()
    {
        auto node = _ui_node->find_from_path(u"Basic.img/Cursor");

        std::unordered_map<Cursor::Event, DynamicSprite> eventsprite;

        auto func = [this, &eventsprite](std::u16string str, Cursor::Event event, wz::Node *node) -> void
        {
            node = node->find_from_path(str);
            if (node != nullptr)
            {
                auto animatedsprite = _sprite_util->load_animated_sprite(node);
                auto dynamicsprite = DynamicSprite(animatedsprite);
                eventsprite.emplace(event, dynamicsprite);
            }
        };

        func(u"0", Cursor::Event::NORMAL, node);
        func(u"12", Cursor::Event::PRESSED, node);

        return eventsprite;
    }
}