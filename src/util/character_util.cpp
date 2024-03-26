#include "util/character_util.hpp"
#include "util/wz_util.hpp"

namespace util
{
    CharacterUtil::CharacterUtil()
    {
        _character_node = WzUtil::current()->Character->get_root();

        _sprite_util = SpriteUtil::current();

        {
            auto body_node = _character_node->find_from_path(u"00002000.img");
            auto head_node = _character_node->find_from_path(u"00012000.img");
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

                        std::unordered_map<Layer, std::unordered_map<std::u16string, Point<int32_t>>> body_shift_map;

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

                                    body_shift_map[z].emplace(map_node.first, Point(v.x, v.y));
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

                                body_shift_map[Layer::HEAD].emplace(map_node.first, Point(v.x, v.y));
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

    std::unordered_map<uint8_t, std::vector<std::tuple<std::vector<Sprite>,int>>> CharacterUtil::load()
    {
        std::unordered_map<uint8_t, std::vector<std::tuple<std::vector<Sprite>, int>>> _s;

        for (uint8_t i = 0; i < Character::Type::LENGTH; i++)
        {
            std::vector<std::tuple<std::vector<Sprite>, int>> _v;
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                std::vector<Sprite> v;
                auto a = body_positions[i][no];
                auto b = arm_positions[i][no];
                auto c = hand_positions[i][no];
                auto d = head_positions[i][no];
                auto e = face_positions[i][no];
                auto f = hair_positions[i][no];

                auto body_node = _character_node->find_from_path(u"00002000.img");
                auto head_node = _character_node->find_from_path(u"00012000.img");

                auto no_str = std::to_string(no);

                auto type = _type_map.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};

                Sprite body = _sprite_util->load_sprite(
                    body_node->find_from_path(type + u"/body"));
                v.push_back(body);

                auto _head = head_node->find_from_path(type + u"/head");
                if (_head != nullptr)
                {
                    Sprite head = _sprite_util->load_sprite(_head, d);
                    v.push_back(head);
                }

                auto _face = _character_node->find_from_path(u"Face/00020000.img/default/face");
                auto _face_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(_face->find_from_path(u"map/brow"))->get();
                Sprite face = _sprite_util->load_sprite(_face, e - Point<int32_t>(_face_pos.x, _face_pos.y));
                v.push_back(face);

                auto _hair = _character_node->find_from_path(u"Hair/00030000.img/" + type + u"/hair");
                if (_hair != nullptr)
                {
                    auto _hair_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(_hair->find_from_path(u"map/brow"))->get();
                    Sprite hair = _sprite_util->load_sprite(_hair, f - Point<int32_t>(_hair_pos.x, _hair_pos.y));
                    v.push_back(hair);
                }

                auto _hair_over_head = _character_node->find_from_path(u"Hair/00030000.img/" + type + u"/hairOverHead");
                if (_hair_over_head != nullptr)
                {
                    auto _hair_over_head_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(_hair_over_head->find_from_path(u"map/brow"))->get();
                    Sprite hair_over_head = _sprite_util->load_sprite(_hair_over_head, f - Point<int32_t>(_hair_over_head_pos.x, _hair_over_head_pos.y));
                    v.push_back(hair_over_head);
                }

                auto _coat = _character_node->find_from_path(u"Coat/01040041.img/" + type + u"/mail");
                if (_coat != nullptr)
                {
                    auto _coat_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(_coat->find_from_path(u"map/navel"))->get();
                    Sprite coat = _sprite_util->load_sprite(_coat, a - Point<int32_t>(_coat_pos.x, _coat_pos.y));
                    v.push_back(coat);
                }

                auto _pants = _character_node->find_from_path(u"Pants/01062040.img/" + type + u"/pants");
                if (_pants != nullptr)
                {
                    auto _pants_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(_pants->find_from_path(u"map/navel"))->get();
                    Sprite pants = _sprite_util->load_sprite(_pants, a - Point<int32_t>(_pants_pos.x, _pants_pos.y));
                    v.push_back(pants);
                }

                auto _arm = body_node->find_from_path(type + u"/arm");
                if (_arm != nullptr)
                {
                    auto _arm_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(_arm->find_from_path(u"map/hand"))->get();
                    Sprite arm = _sprite_util->load_sprite(_arm, b - Point<int32_t>(_arm_pos.x, _arm_pos.y));
                    v.push_back(arm);
                }

                auto _coat_arm = _character_node->find_from_path(u"Coat/01040041.img/" + type + u"/mailArm");
                if (_coat_arm != nullptr)
                {
                    auto _coat_arm_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(_coat_arm->find_from_path(u"map/navel"))->get();
                    Sprite coat_arm = _sprite_util->load_sprite(_coat_arm, a - Point<int32_t>(_coat_arm_pos.x, _coat_arm_pos.y));
                    v.push_back(coat_arm);
                }

                auto _shoes = _character_node->find_from_path(u"Shoes/01070009.img/" + type + u"/shoes");
                if (_shoes != nullptr)
                {
                    auto _shoes_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(_shoes->find_from_path(u"map/navel"))->get();
                    Sprite shoes = _sprite_util->load_sprite(_shoes, a - Point<int32_t>(_shoes_pos.x, _shoes_pos.y));
                    v.push_back(shoes);
                }

                auto delay = stance_delays[i][no];

                _v.push_back({v, delay});
            }
            _s.emplace(i,_v);
        }
        return _s;
    }
}
