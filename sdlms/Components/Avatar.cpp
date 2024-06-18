#include "Avatar.h"
#include "Resource/Wz.h"

Avatar *Avatar::load()
{
    [[unlikely]]
    if (!inited)
    {
        inited = true;
        character_node = World::get_world()->get_resource<Wz>().Character->get_root();
        init(character_node);
    }
    return new Avatar();
}

void Avatar::init(wz::Node *character_node)
{
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
            Sprite *sprite = new Sprite(body_node->find_from_path(type + u"/body"));
            Transform *f = new Transform();
            body[i][no] = {f, sprite};
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
                if (coat_node->find_from_path(type + u"/mail") != nullptr)
                {
                    auto coat_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(coat_node->find_from_path(type + u"/mail/map/navel"))->get();
                    Sprite *sprite = new Sprite(coat_node->find_from_path(type + u"/mail"));
                    Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)coat_pos.x, (float)coat_pos.y});
                    coat[i][no] = {f, sprite};
                }
            }
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
                    pants[i][no] = {f, sprite};
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
                head[i][no] = {f, sprite};
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
                face[i][no] = {f, sprite};
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
                    auto hair = it.second[0];
                    if (hair->type == wz::Type::UOL)
                    {
                        hair = dynamic_cast<wz::Property<wz::WzUOL> *>(hair)->get_uol();
                    }
                    if (it.first == u"hairShade")
                    {
                        hair = hair->find_from_path(u"0");
                    }
                    auto hair_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(hair->find_from_path(u"map/brow"))->get();
                    Sprite *sprite = new Sprite(hair);
                    Transform *f = new Transform(face_positions[i][no] - SDL_FPoint{(float)hair_pos.x, (float)hair_pos.y});
                    hairs[i][no].push_back({f, sprite});
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
                    shoes[i][no] = {f, sprite};
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
                        weapon[i][no] = {f, sprite};
                    }
                    else
                    {
                        auto weapon_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(weapon_node->find_from_path(type + u"/weapon/map/navel"))->get();
                        Sprite *sprite = new Sprite(weapon_node->find_from_path(type + u"/weapon"));
                        Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)weapon_pos.x, (float)weapon_pos.y});
                        weapon[i][no] = {f, sprite};
                    }
                }
            }
        }
    }
}

void Avatar::add_arm(const std::u16string &val)
{
    auto body_node = character_node->find_from_path(val + u".img");
    for (uint8_t i = 0; i < ACTION::LENGTH; i++)
    {
        for (uint8_t no = 0; no < body_positions[i].size(); no++)
        {
            auto no_str = std::to_string(no);
            auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
            if (body_node->find_from_path(type + u"/arm") != nullptr)
            {
                auto arm_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(body_node->find_from_path(type + u"/arm/map/hand"))->get();
                Sprite *sprite = new Sprite(body_node->find_from_path(type + u"/arm"));
                Transform *f = new Transform(arm_positions[i][no] - SDL_FPoint{(float)arm_pos.x, (float)arm_pos.y});
                arm[i][no] = {f, sprite};
            }
        }
    }
}

void Avatar::add_coat_arm(const std::u16string &val)
{
    auto coat_arm_node = character_node->find_from_path(u"Coat/" + val + u".img");
    if (coat_arm_node != nullptr)
    {
        for (uint8_t i = 0; i < ACTION::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto no_str = std::to_string(no);
                auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
                if (coat_arm_node->find_from_path(type + u"/mailArm") != nullptr)
                {
                    auto coat_arm_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(coat_arm_node->find_from_path(type + u"/mailArm/map/navel"))->get();
                    Sprite *sprite = new Sprite(coat_arm_node->find_from_path(type + u"/mailArm"));
                    Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)coat_arm_pos.x, (float)coat_arm_pos.y});
                    coat_arm[i][no] = {f, sprite};
                }
            }
        }
    }
}

void Avatar::add_hand(const std::u16string &val)
{
    auto body_node = character_node->find_from_path(val + u".img");
    for (uint8_t i = 0; i < ACTION::LENGTH; i++)
    {
        for (uint8_t no = 0; no < body_positions[i].size(); no++)
        {
            auto no_str = std::to_string(no);
            auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
            if (body_node->find_from_path(type + u"/hand") != nullptr)
            {
                auto _hand = body_node->find_from_path(type + u"/hand");
                if (_hand->find_from_path(u"map/navel") != nullptr)
                {
                    auto hand_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(_hand->find_from_path(u"map/navel"))->get();
                    Sprite *sprite = new Sprite(_hand);
                    Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)hand_pos.x, (float)hand_pos.y});
                    hand[i][no] = {f, sprite};
                }
            }
        }
    }
}

void Avatar::add_lHand(const std::u16string &val)
{
    auto body_node = character_node->find_from_path(val + u".img");
    for (uint8_t i = 0; i < ACTION::LENGTH; i++)
    {
        for (uint8_t no = 0; no < body_positions[i].size(); no++)
        {
            auto no_str = std::to_string(no);
            auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
            if (body_node->find_from_path(type + u"/lHand") != nullptr)
            {
                auto _lHand = body_node->find_from_path(type + u"/lHand");
                if (_lHand->find_from_path(u"map/handMove") != nullptr)
                {
                    auto lHand_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(_lHand->find_from_path(u"map/handMove"))->get();
                    Sprite *sprite = new Sprite(_lHand);
                    Transform *f = new Transform(hand_positions[i][no] - SDL_FPoint{(float)lHand_pos.x, (float)lHand_pos.y});
                    lHand[i][no] = {f, sprite};
                }
                else
                {
                    auto lHand_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(_lHand->find_from_path(u"map/navel"))->get();
                    Sprite *sprite = new Sprite(_lHand);
                    Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)lHand_pos.x, (float)lHand_pos.y});
                    lHand[i][no] = {f, sprite};
                }
            }
        }
    }
}

void Avatar::add_rHand(const std::u16string &val)
{
    auto body_node = character_node->find_from_path(val + u".img");
    for (uint8_t i = 0; i < ACTION::LENGTH; i++)
    {
        for (uint8_t no = 0; no < body_positions[i].size(); no++)
        {
            auto no_str = std::to_string(no);
            auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
            if (body_node->find_from_path(type + u"/rHand") != nullptr)
            {
                auto _rHand = body_node->find_from_path(type + u"/rHand");
                if (_rHand->find_from_path(u"map/navel") != nullptr)
                {
                    auto rHand_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(_rHand->find_from_path(u"map/navel"))->get();
                    Sprite *sprite = new Sprite(_rHand);
                    Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)rHand_pos.x, (float)rHand_pos.y});
                    rHand[i][no] = {f, sprite};
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
    for (auto &it : coat)
    {
        for (auto &[key, val] : it)
        {
            del_func(val);
        }
    }
    for (auto &it : pants)
    {
        for (auto &[key, val] : it)
        {
            del_func(val);
        }
    }
    for (auto &it : head)
    {
        for (auto &[key, val] : it)
        {
            del_func(val);
        }
    }
    for (auto &it : face)
    {
        for (auto &[key, val] : it)
        {
            del_func(val);
        }
    }
    for (auto &it : hairs)
    {
        for (auto &[key, val] : it)
        {
            for (auto &pai : val)
            {
                del_func(pai);
            }
        }
    }
    for (auto &it : coat_arm)
    {
        for (auto &[key, val] : it)
        {
            del_func(val);
        }
    }
    for (auto &it : weapon)
    {
        for (auto &[key, val] : it)
        {
            del_func(val);
        }
    }
}