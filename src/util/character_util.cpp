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

                        std::unordered_map<Layer, std::unordered_map<std::u16string, std::tuple<int, int>>> body_shift_map;

                        for (auto part_node : frame_node->get_children())
                        {
                            auto part = part_node.first;

                            if (part != u"delay" && part != u"face")
                            {
                                auto zstr = dynamic_cast<wz::Property<wz::wzstring> *>(part_node.second[0]->get_child(u"z"))->get();
                                auto z = layer_map.at(zstr);

                                for (auto map_node : part_node.second[0]->get_child(u"map")->get_children())
                                {
                                    auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(map_node.second[0])->get();

                                    body_shift_map[z].emplace(map_node.first, std::tuple<int, int>{v.x, v.y});
                                }
                            }
                        }
                        std::string frame_str = std::to_string(frame);
                        auto h = head_node->find_from_path(ststr + u"/" + std::u16string{frame_str.begin(), frame_str.end()} + u"/" + u"head");
                        if (h->type == wz::Type::UOL)
                        {
                            h = dynamic_cast<wz::Property<wz::WzUOL> *>(h)->get_uol();
                        }
                    }
                }
            }
        }
    }

    std::map<Character::Status, DynamicSprite> CharacterUtil::load()
    {
    }

    void CharacterUtil::load_body(wz::Node *node)
    {
    }
}
