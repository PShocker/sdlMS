#include "Install.h"
#include "wz/Property.hpp"
#include "Resources/Wz.h"

Install::Install(const std::u16string &id) : id(id)
{
    auto node = Wz::Item->get_root()->find_from_path(u"Install/" + id.substr(0, id.length() - 4) + u".img/" + id);
    if (node->get_child(u"effect"))
    {
        aspr = AnimatedSprite(AnimatedSprite::Wrap::load(node->get_child(u"effect")));
    }
}
