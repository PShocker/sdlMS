#include "Install.h"
#include "wz/Property.hpp"
#include "Resources/Wz.h"

Install::Install(int id) : id(id)
{
    std::string str = std::to_string(id);
    std::u16string id_str(str.begin(), str.end());
    id_str = u"0" + id_str;

    auto node = Wz::Item->get_root()->find_from_path(u"Install/" + id_str.substr(0, id_str.length() - 4) + u".img/" + id_str);
    if (node->get_child(u"effect"))
    {
        aspr = AnimatedSprite(AnimatedSpriteWarp::load(node->get_child(u"effect")));
    }
}
