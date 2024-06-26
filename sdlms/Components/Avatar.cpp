#include "Avatar.h"
#include "Resource/Wz.h"
#include <ranges>

Avatar *Avatar::load()
{
    [[unlikely]]
    if (!inited)
    {
        inited = true;
        init();
    }
    return new Avatar();
}

void Avatar::init()
{
    character_node = World::get_world()->get_resource<Wz>().Character->get_root();
    {
        auto body_node = character_node->find_from_path(u"00002000.img");
        auto head_node = character_node->find_from_path(u"00012000.img");
        for (auto stance_node : body_node->get_children())
        {
            auto ststr = stance_node.first;

            uint16_t attackdelay = 0;

            for (uint8_t frame = 0; auto frame_node = stance_node.second[0]->get_child(std::to_string(frame)); ++frame)
            {
                if (frame_node->get_child(u"action") != nullptr)
                {
                    /* code */
                }
                else
                {
                    auto type = type_map.at(ststr);
                    auto delay_node = frame_node->get_child(u"delay");
                    auto delay = 100;
                    if (delay_node != nullptr)
                    {
                        delay = dynamic_cast<wz::Property<int> *>(delay_node)->get();
                    }
                    stance_delays[type][frame] = delay;

                    auto face_node = frame_node->get_child(u"face");
                    auto face = true;
                    if (face_node != nullptr)
                    {
                        face = dynamic_cast<wz::Property<unsigned short> *>(face_node)->get();
                    }
                    show_face[type][frame] = face;

                    std::unordered_map<Layer, std::unordered_map<std::u16string, SDL_FPoint>> body_shift_map;

                    for (auto part_node : frame_node->get_children())
                    {
                        auto part_name = part_node.first;

                        if (part_name != u"delay" && part_name != u"face")
                        {
                            auto part = part_node.second[0];
                            if (part->type == wz::Type::UOL)
                            {
                                part = dynamic_cast<wz::Property<wz::WzUOL> *>(part)->get_uol();
                            }

                            auto zstr = dynamic_cast<wz::Property<wz::wzstring> *>(part->get_child(u"z"))->get();
                            auto z = layer_map.at(zstr);

                            for (auto map_node : part->get_child(u"map")->get_children())
                            {
                                auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(map_node.second[0])->get();

                                body_shift_map[z].emplace(map_node.first, SDL_FPoint{(float)v.x, (float)v.y});
                            }
                        }
                    }
                    std::string frame_str = std::to_string(frame);
                    auto h = head_node->find_from_path(ststr + u"/" + std::u16string{frame_str.begin(), frame_str.end()} + u"/" + u"head");
                    if (h != nullptr)
                    {
                        if (h->type == wz::Type::UOL)
                        {
                            h = dynamic_cast<wz::Property<wz::WzUOL> *>(h)->get_uol();
                        }
                        for (auto map_node : h->get_child(u"map")->get_children())
                        {
                            auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(map_node.second[0])->get();

                            body_shift_map[Layer::HEAD].emplace(map_node.first, SDL_FPoint{(float)v.x, (float)v.y});
                        }
                    }

                    body_positions[type][frame] = body_shift_map[Layer::BODY][u"navel"];

                    arm_positions[type][frame] = body_shift_map.count(Layer::ARM) ? (body_shift_map[Layer::ARM][u"hand"] - body_shift_map[Layer::ARM][u"navel"] + body_shift_map[Layer::BODY][u"navel"])
                                                                                  : (body_shift_map[Layer::ARM_OVER_HAIR][u"hand"] - body_shift_map[Layer::ARM_OVER_HAIR][u"navel"] + body_shift_map[Layer::BODY][u"navel"]);
                    hand_positions[type][frame] = body_shift_map[Layer::HAND_BELOW_WEAPON][u"handMove"];
                    head_positions[type][frame] = body_shift_map[Layer::BODY][u"neck"] - body_shift_map[Layer::HEAD][u"neck"];
                    face_positions[type][frame] = body_shift_map[Layer::BODY][u"neck"] - body_shift_map[Layer::HEAD][u"neck"] + body_shift_map[Layer::HEAD][u"brow"];
                    hair_positions[type][frame] = body_shift_map[Layer::HEAD][u"brow"] - body_shift_map[Layer::HEAD][u"neck"] + body_shift_map[Layer::BODY][u"neck"];
                }
            }
        }
    }
}

void Avatar::add_body(const std::u16string &val)
{
    auto body_node = character_node->find_from_path(val + u".img");
    for (uint8_t i = 0; i < ACTION::LENGTH; i++)
    {
        for (uint8_t no = 0; no < body_positions[i].size(); no++)
        {
            auto no_str = std::to_string(no);
            auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
            {
                Sprite *sprite = new Sprite(body_node->find_from_path(type + u"/body"));
                Transform *f = new Transform();
                auto z = std::any_cast<std::u16string>(sprite->get_z());
                auto part = *zmap[z];
                part[i][no] = {f, sprite};
            }
            {
                if (body_node->find_from_path(type + u"/arm") != nullptr)
                {
                    auto arm_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(body_node->find_from_path(type + u"/arm/map/hand"))->get();
                    Sprite *sprite = new Sprite(body_node->find_from_path(type + u"/arm"));
                    Transform *f = new Transform(arm_positions[i][no] - SDL_FPoint{(float)arm_pos.x, (float)arm_pos.y});
                    auto z = std::any_cast<std::u16string>(sprite->get_z());
                    auto part = *zmap[z];
                    part[i][no] = {f, sprite};
                }
            }
            {
                if (body_node->find_from_path(type + u"/hand") != nullptr)
                {
                    auto hand = body_node->find_from_path(type + u"/hand");
                    if (body_node->find_from_path(type + u"/hand/map/navel") != nullptr)
                    {
                        auto hand_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(hand->find_from_path(u"map/navel"))->get();
                        Sprite *sprite = new Sprite(hand);
                        Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)hand_pos.x, (float)hand_pos.y});
                        auto z = std::any_cast<std::u16string>(sprite->get_z());
                        auto part = *zmap[z];
                        part[i][no] = {f, sprite};
                    }
                }
            }
            {
                if (body_node->find_from_path(type + u"/lHand") != nullptr)
                {
                    auto lHand = body_node->find_from_path(type + u"/lHand");
                    if (lHand->find_from_path(u"map/handMove") != nullptr)
                    {
                        auto lHand_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(lHand->find_from_path(u"map/handMove"))->get();
                        Sprite *sprite = new Sprite(lHand);
                        Transform *f = new Transform(hand_positions[i][no] - SDL_FPoint{(float)lHand_pos.x, (float)lHand_pos.y});
                        auto z = std::any_cast<std::u16string>(sprite->get_z());
                        auto part = *zmap[z];
                        part[i][no] = {f, sprite};
                    }
                    else
                    {
                        auto lHand_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(lHand->find_from_path(u"map/navel"))->get();
                        Sprite *sprite = new Sprite(lHand);
                        Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)lHand_pos.x, (float)lHand_pos.y});
                        auto z = std::any_cast<std::u16string>(sprite->get_z());
                        auto part = *zmap[z];
                        part[i][no] = {f, sprite};
                    }
                }
            }
            {
                if (body_node->find_from_path(type + u"/rHand") != nullptr)
                {
                    auto rHand = body_node->find_from_path(type + u"/rHand");
                    if (rHand->find_from_path(u"map/navel") != nullptr)
                    {
                        auto rHand_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(rHand->find_from_path(u"map/navel"))->get();
                        Sprite *sprite = new Sprite(rHand);
                        Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)rHand_pos.x, (float)rHand_pos.y});
                        auto z = std::any_cast<std::u16string>(sprite->get_z());
                        auto part = *zmap[z];
                        part[i][no] = {f, sprite};
                    }
                }
            }
        }
    }
}

void Avatar::add_coat(const std::u16string &val)
{
    auto coat_node = character_node->find_from_path(u"Coat/" + val + u".img");
    if (coat_node != nullptr)
    {
        for (uint8_t i = 0; i < ACTION::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto no_str = std::to_string(no);
                auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
                {
                    if (coat_node->find_from_path(type + u"/mail") != nullptr)
                    {
                        auto coat_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(coat_node->find_from_path(type + u"/mail/map/navel"))->get();
                        Sprite *sprite = new Sprite(coat_node->find_from_path(type + u"/mail"));
                        Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)coat_pos.x, (float)coat_pos.y});
                        auto z = std::any_cast<std::u16string>(sprite->get_z());
                        auto part = *zmap[z];
                        part[i][no] = {f, sprite};
                    }
                }
                {
                    if (coat_node->find_from_path(type + u"/mailArm") != nullptr)
                    {
                        auto mail_arm_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(coat_node->find_from_path(type + u"/mailArm/map/navel"))->get();
                        Sprite *sprite = new Sprite(coat_node->find_from_path(type + u"/mailArm"));
                        Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)mail_arm_pos.x, (float)mail_arm_pos.y});
                        auto z = std::any_cast<std::u16string>(sprite->get_z());
                        auto part = *zmap[z];
                        part[i][no] = {f, sprite};
                    }
                }
            }
        }
    }
}

void Avatar::add_cap(const std::u16string &val)
{
    auto cap_node = character_node->find_from_path(u"Cap/" + val + u".img");
    if (cap_node != nullptr)
    {
        cap_vslot.clear();
        for (uint8_t i = 0; i < ACTION::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto no_str = std::to_string(no);
                auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
                if (cap_node->find_from_path(type) != nullptr)
                {
                    for (auto it : cap_node->find_from_path(type)->get_children())
                    {
                        auto cap = it.second[0];
                        if (cap->type == wz::Type::UOL)
                        {
                            cap = dynamic_cast<wz::Property<wz::WzUOL> *>(cap)->get_uol();
                        }
                        auto cap_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(cap->find_from_path(u"map/brow"))->get();
                        Sprite *sprite = new Sprite(cap);
                        Transform *f = new Transform(face_positions[i][no] - SDL_FPoint{(float)cap_pos.x, (float)cap_pos.y});
                        auto z = std::any_cast<std::u16string>(sprite->get_z());
                        auto part = *zmap[z];
                        part[i][no] = {f, sprite};
                    }
                }
            }
        }
        auto vslot = dynamic_cast<wz::Property<wz::wzstring> *>(cap_node->find_from_path(u"info/vslot"))->get();
        for (size_t i = 0; i < vslot.size(); i += 2)
        {
            cap_vslot.emplace(vslot.substr(i, 2));
        }
    }
}

void Avatar::add_pants(const std::u16string &val)
{
    auto pants_node = character_node->find_from_path(u"Pants/" + val + u".img");
    if (pants_node != nullptr)
    {
        for (uint8_t i = 0; i < ACTION::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto no_str = std::to_string(no);
                auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
                if (pants_node->find_from_path(type + u"/pants") != nullptr)
                {
                    auto pants_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(pants_node->find_from_path(type + u"/pants/map/navel"))->get();
                    Sprite *sprite = new Sprite(pants_node->find_from_path(type + u"/pants"));
                    Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)pants_pos.x, (float)pants_pos.y});
                    auto z = std::any_cast<std::u16string>(sprite->get_z());
                    auto part = *zmap[z];
                    part[i][no] = {f, sprite};
                }
            }
        }
    }
}

void Avatar::add_head(const std::u16string &val)
{
    auto head_node = character_node->find_from_path(val + u".img");
    for (uint8_t i = 0; i < ACTION::LENGTH; i++)
    {
        for (uint8_t no = 0; no < body_positions[i].size(); no++)
        {
            auto no_str = std::to_string(no);
            auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
            if (head_node->find_from_path(type + u"/head") != nullptr)
            {
                Sprite *sprite = new Sprite(head_node->find_from_path(type + u"/head"));
                Transform *f = new Transform(head_positions[i][no]);
                auto z = std::any_cast<std::u16string>(sprite->get_z());
                auto part = *zmap[z];
                part[i][no] = {f, sprite};
            }
        }
    }
}

void Avatar::add_face(const std::u16string &val)
{
    auto face_node = character_node->find_from_path(u"Face/" + val + u".img/default/face");
    if (face_node != nullptr)
    {
        for (uint8_t i = 0; i < ACTION::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto face_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(face_node->find_from_path(u"map/brow"))->get();
                Sprite *sprite = new Sprite(face_node);
                Transform *f = new Transform(face_positions[i][no] - SDL_FPoint{(float)face_pos.x, (float)face_pos.y});
                auto z = std::any_cast<std::u16string>(sprite->get_z());
                auto part = *zmap[z];
                part[i][no] = {f, sprite};
            }
        }
    }
}

void Avatar::add_hairs(const std::u16string &val)
{
    for (uint8_t i = 0; i < ACTION::LENGTH; i++)
    {
        for (uint8_t no = 0; no < body_positions[i].size(); no++)
        {
            auto no_str = std::to_string(no);
            auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
            auto hairs_node = character_node->find_from_path(u"Hair/" + val + u".img/" + type);
            if (hairs_node != nullptr)
            {
                for (auto it : hairs_node->get_children())
                {
                    auto hairs = it.second[0];
                    if (hairs->type == wz::Type::UOL)
                    {
                        hairs = dynamic_cast<wz::Property<wz::WzUOL> *>(hairs)->get_uol();
                    }
                    if (it.first == u"hairShade")
                    {
                        hairs = hairs->find_from_path(u"0");
                    }
                    auto hair_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(hairs->find_from_path(u"map/brow"))->get();
                    Sprite *sprite = new Sprite(hairs);
                    Transform *f = new Transform(face_positions[i][no] - SDL_FPoint{(float)hair_pos.x, (float)hair_pos.y});
                    auto z = std::any_cast<std::u16string>(sprite->get_z());
                    auto part = *zmap[z];
                    part[i][no] = {f, sprite};
                }
            }
        }
    }
}

void Avatar::add_shoes(const std::u16string &val)
{
    auto shoes_node = character_node->find_from_path(u"Shoes/" + val + u".img");
    if (shoes_node != nullptr)
    {
        for (uint8_t i = 0; i < ACTION::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto no_str = std::to_string(no);
                auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
                if (shoes_node->find_from_path(type + u"/shoes") != nullptr)
                {
                    auto shoes_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(shoes_node->find_from_path(type + u"/shoes/map/navel"))->get();
                    Sprite *sprite = new Sprite(shoes_node->find_from_path(type + u"/shoes"));
                    Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)shoes_pos.x, (float)shoes_pos.y});
                    auto z = std::any_cast<std::u16string>(sprite->get_z());
                    auto part = *zmap[z];
                    part[i][no] = {f, sprite};
                }
            }
        }
    }
}

void Avatar::add_weapon(const std::u16string &val)
{
    auto weapon_node = character_node->find_from_path(u"Weapon/" + val + u".img");
    if (weapon_node != nullptr)
    {
        for (uint8_t i = 0; i < ACTION::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto no_str = std::to_string(no);
                auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
                if (weapon_node->find_from_path(type + u"/weapon") != nullptr)
                {
                    if (weapon_node->find_from_path(type + u"/weapon/map/hand") != nullptr)
                    {
                        auto weapon_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(weapon_node->find_from_path(type + u"/weapon/map/hand"))->get();
                        Sprite *sprite = new Sprite(weapon_node->find_from_path(type + u"/weapon"));
                        Transform *f = new Transform(arm_positions[i][no] - SDL_FPoint{(float)weapon_pos.x, (float)weapon_pos.y});
                        auto z = std::any_cast<std::u16string>(sprite->get_z());
                        auto part = *zmap[z];
                        part[i][no] = {f, sprite};
                    }
                    else
                    {
                        auto weapon_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(weapon_node->find_from_path(type + u"/weapon/map/navel"))->get();
                        Sprite *sprite = new Sprite(weapon_node->find_from_path(type + u"/weapon"));
                        Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)weapon_pos.x, (float)weapon_pos.y});
                        auto z = std::any_cast<std::u16string>(sprite->get_z());
                        auto part = *zmap[z];
                        part[i][no] = {f, sprite};
                    }
                }
            }
        }
    }
}

void Avatar::add_shield(const std::u16string &val)
{
    auto shield_node = character_node->find_from_path(u"Shield/" + val + u".img");
    if (shield_node != nullptr)
    {
        for (uint8_t i = 0; i < ACTION::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto no_str = std::to_string(no);
                auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
                if (shield_node->find_from_path(type + u"/shield") != nullptr)
                {
                    auto shield_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(shield_node->find_from_path(type + u"/shield/map/navel"))->get();
                    Sprite *sprite = new Sprite(shield_node->find_from_path(type + u"/shield"));
                    Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)shield_pos.x, (float)shield_pos.y});
                    auto z = std::any_cast<std::u16string>(sprite->get_z());
                    auto part = *zmap[z];
                    part[i][no] = {f, sprite};
                }
            }
        }
    }
}

Avatar::~Avatar()
{
    auto del_func = [](std::pair<Transform *, Sprite *> &pai) -> void
    {
        auto &[tr, spr] = pai;
        delete tr;
        delete spr;
    };
    for (auto &it : body)
    {
        for (auto &[key, val] : it)
        {
            del_func(val);
        }
    }
}