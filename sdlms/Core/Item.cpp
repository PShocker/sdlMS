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

uint8_t Item::load_item_index(std::u16string item_id)
{
    auto prefix = item_id[0];
    switch (prefix)
    {
    case u'1':
    {
        return 0;
    }
    break;
    case u'2':
    {
        return 1;
    }
    break;
    case u'3':
    {
        return 2;
    }
    break;
    case u'4':
    {
        return 3;
    }
    break;
    case u'5':
    {
        return 4;
    }
    break;
    default:
        break;
    }
    return 4;
}

wz::Node *Item::load_item_string(std::u16string item_id)
{
    auto node = Wz::String->get_root();
    switch (load_item_index(item_id))
    {
    case 0:
    {
        // 装备类物品,需要细分类别
        auto id = std::stoi(std::string{item_id.begin(), item_id.end()});
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
        node = node->find_from_path(u"Eqp.img/Eqp/" + type + u"/" + item_id);
    }
    break;
    case 1:
    {
        node = node->find_from_path(u"Consume.img/" + item_id);
    }
    break;
    case 2:
    {
        node = node->find_from_path(u"Ins.img/" + item_id);
    }
    break;
    case 3:
    {
        node = node->find_from_path(u"Etc.img/Etc/" + item_id);
    }
    break;
    case 4:
    {
        // 特殊类物品
        auto prefix_str = item_id.substr(0, 4);
        if (prefix_str == u"0500")
        {
            node = node->find_from_path(u"Pet.img/" + item_id);
        }
        else
        {
            node = node->find_from_path(u"Cash.img/" + item_id);
        }
    }
    break;
    default:
        break;
    }
    return node;
}

std::u16string Item::load_item_name(std::u16string item_id)
{
    auto node = load_item_string(item_id);
    return dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"name"))->get();
}
