#include "Avatar.h"
#include "Resource/Wz.h"

Avatar *Avatar::load_avatar(wz::Node *character_node)
{
    if (!inited)
    {
        inited = true;
        chara_node = character_node;
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

void Avatar::switch_act(uint8_t a)
{
    if (a != act)
    {
        act_index = 0;
        act_time = 0;
        act = a;
    }
}

void Avatar::add_body()
{
    auto body_node = chara_node->find_from_path(u"00002000.img");

    for (uint8_t i = 0; i < ACTION::LENGTH; i++)
    {
        for (uint8_t no = 0; no < body_positions[i].size(); no++)
        {
            auto no_str = std::to_string(no);
            auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
            Sprite *sprite = Sprite::load_sprite(body_node->find_from_path(type + u"/body"));
            Transform *f = new Transform();
            body[i][no] = {f, sprite};
        }
    }
}

void Avatar::add_coat(std::u16string val)
{
    auto coat_node = chara_node->find_from_path(u"Coat/" + val + u".img");
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
                    Sprite *sprite = Sprite::load_sprite(coat_node->find_from_path(type + u"/mail"));
                    Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)coat_pos.x, (float)coat_pos.y});
                    coat[i][no] = {f, sprite};
                }
            }
        }
    }
}

void Avatar::add_pants(std::u16string val)
{
    auto pants_node = chara_node->find_from_path(u"Pants/" + val + u".img");
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
                    Sprite *sprite = Sprite::load_sprite(pants_node->find_from_path(type + u"/pants"));
                    Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)pants_pos.x, (float)pants_pos.y});
                    pants[i][no] = {f, sprite};
                }
            }
        }
    }
}

void Avatar::add_head()
{
    auto head_node = chara_node->find_from_path(u"00012000.img");

    for (uint8_t i = 0; i < ACTION::LENGTH; i++)
    {
        for (uint8_t no = 0; no < body_positions[i].size(); no++)
        {
            auto no_str = std::to_string(no);
            auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
            if (head_node->find_from_path(type + u"/head") != nullptr)
            {
                Sprite *sprite = Sprite::load_sprite(head_node->find_from_path(type + u"/head"));
                Transform *f = new Transform(head_positions[i][no]);
                head[i][no] = {f, sprite};
            }
        }
    }
}

void Avatar::add_face(std::u16string val)
{
    auto face_node = chara_node->find_from_path(u"Face/" + val + u".img/default/face");
    if (face_node != nullptr)
    {
        for (uint8_t i = 0; i < ACTION::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto face_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(face_node->find_from_path(u"map/brow"))->get();
                Sprite *sprite = Sprite::load_sprite(face_node);
                Transform *f = new Transform(face_positions[i][no] - SDL_FPoint{(float)face_pos.x, (float)face_pos.y});
                face[i][no] = {f, sprite};
            }
        }
    }
}

void Avatar::add_hairs(std::u16string val)
{

    for (uint8_t i = 0; i < ACTION::LENGTH; i++)
    {
        for (uint8_t no = 0; no < body_positions[i].size(); no++)
        {
            auto no_str = std::to_string(no);
            auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
            auto hairs_node = chara_node->find_from_path(u"Hair/" + val + u".img/" + type);
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
                    Sprite *sprite = Sprite::load_sprite(hair);
                    Transform *f = new Transform(face_positions[i][no] - SDL_FPoint{(float)hair_pos.x, (float)hair_pos.y});
                    hairs[i][no].push_back({f, sprite});
                }
            }
        }
    }
}

void Avatar::add_shoes(std::u16string val)
{
}

void Avatar::add_weapon(std::u16string val)
{
}

void Avatar::add_arm()
{
    auto body_node = chara_node->find_from_path(u"00002000.img");

    for (uint8_t i = 0; i < ACTION::LENGTH; i++)
    {
        for (uint8_t no = 0; no < body_positions[i].size(); no++)
        {
            auto no_str = std::to_string(no);
            auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
            if (body_node->find_from_path(type + u"/arm") != nullptr)
            {
                auto arm_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(body_node->find_from_path(type + u"/arm/map/hand"))->get();
                Sprite *sprite = Sprite::load_sprite(body_node->find_from_path(type + u"/arm"));
                Transform *f = new Transform(arm_positions[i][no] - SDL_FPoint{(float)arm_pos.x, (float)arm_pos.y});
                arm[i][no] = {f, sprite};
            }
        }
    }
}

void Avatar::add_hand()
{
}

void Avatar::add_lHand()
{
    auto body_node = chara_node->find_from_path(u"00002000.img");

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
                    Sprite *sprite = Sprite::load_sprite(_lHand);
                    Transform *f = new Transform(hand_positions[i][no] - SDL_FPoint{(float)lHand_pos.x, (float)lHand_pos.y});
                    lHand[i][no] = {f, sprite};
                }
                else
                {
                    auto lHand_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(_lHand->find_from_path(u"map/navel"))->get();
                    Sprite *sprite = Sprite::load_sprite(_lHand);
                    Transform *f = new Transform(body_positions[i][no] - SDL_FPoint{(float)lHand_pos.x, (float)lHand_pos.y});
                    lHand[i][no] = {f, sprite};
                }
            }
        }
    }
}

void Avatar::add_rHand()
{
}
