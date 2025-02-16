#include "Item.h"
#include "Resources/Wz.h"

const std::u16string item_path[] =
    {
        u"Cap",
        u"Accessory",
        u"Accessory",
        u"Accessory",
        u"Coat",
        u"Longcoat",
        u"Pants",
        u"Shoes",
        u"Glove",
        u"Shield",
        u"Cape",
        u"Ring",
        u"Accessory",
        u"Accessory",
        u"Accessory"};

wz::Node *Item::load(std::u16string item_id)
{
    auto prefix = item_id[0];
    auto id = std::stoi(std::string{item_id.begin(), item_id.end()});
    switch (prefix)
    {
    case u'0':
    {
        if (id >= 20000 && id < 30000)
        {
            return Wz::Character->get_root()->find_from_path(u"Face/" + item_id + u".img");
        }
        else
        {
            return Wz::Character->get_root()->find_from_path(u"Hair/" + item_id + u".img");
        }
        //  strPath = $"String/Eqp.img/Eqp/{Category}/{id}";
    }
    break;
    case u'1':
    {
        item_id = u"0" + item_id;
        auto index = (id / 10000) - 100;
        std::u16string type;
        if (index < 15)
        {
            type = item_path[index];
        }
        else if (index >= 30 && index <= 70)
        {
            type = u"Weapon";
        }
        return Wz::Character->get_root()->find_from_path(type + u"/" + item_id + u".img");
    }
    break;
    case u'2':
    {
        item_id = u"0" + item_id;
        auto prefix_str = item_id.substr(0, 4);
        return Wz::Item->get_root()->find_from_path(u"Consume/" + prefix_str + u".img/" + item_id);
    }
    break;
    case u'3':
    {
        auto prefix_str = item_id.substr(0, 4);
        return Wz::Item->get_root()->find_from_path(u"Install/" + prefix_str + u".img/" + item_id);
    }
    break;
    case u'4':
    {
        item_id = u"0" + item_id;
        auto prefix_str = item_id.substr(0, 4);
        return Wz::Item->get_root()->find_from_path(u"Etc/" + prefix_str + u".img/" + item_id);
    }
    break;
    case u'5':
    {
        auto prefix_str = item_id.substr(0, 4);
        if (prefix_str == u"0500")
        {
            return Wz::Item->get_root()->find_from_path(u"Pet/" + item_id + u".img");
        }
        else
        {
            return Wz::Item->get_root()->find_from_path(u"Cash/" + prefix_str + u".img/" + item_id);
        }
    }
    break;
    default:
        break;
    }
    return nullptr;
}
