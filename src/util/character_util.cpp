#include "util/character_util.hpp"
#include "util/wz_util.hpp"

namespace util
{
    CharacterUtil::CharacterUtil()
    {
        _character_node = WzUtil::current()->Character->get_root();

        _sprite_util = SpriteUtil::current();
    }

    std::map<Character::Status, DynamicSprite> CharacterUtil::load()
    {
        auto node = _character_node->find_from_path(u"2003.img");

        std::map<Character::Status, DynamicSprite> eventsprite;

        auto func = [this, &eventsprite](std::u16string str, Character::Status event, wz::Node *node) -> void
        {
            node = node->find_from_path(str);
            if (node != nullptr)
            {
                auto animatedsprite = _sprite_util->load_animated_sprite(node);
                auto dynamicsprite = DynamicSprite(animatedsprite);
                eventsprite.emplace(event, dynamicsprite);
            }
        };

        // func(u"0", Character::Status::NORMAL, node);
        // func(u"12", Character::Status::PRESSED, node);

        return eventsprite;
    }

    void CharacterUtil::load_body(wz::Node *node)
    {
        for (auto it : node->get_children())
        {
            if (it.first != u"info")
            {
                auto action = it.second[0];
                auto body = action->get_child(u"body");
                

                for (auto it : action->get_children())
                {
                    if (it.first != u"body")
                    {

                    }
                }
            }
        }
    }
}
