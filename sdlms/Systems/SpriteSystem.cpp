#include <vector>
#include "SpriteSystem.h"
#include "Components/Player.h"

void SpriteSystem::run(World &world)
{
    if (world.components_exist_of_type<DistanceSprite>())
    {
        for (auto &[index, dis] : world.get_components<DistanceSprite>())
        {
            update_dis(dis, world);
        }
    }
}

void SpriteSystem::update_dis(DistanceSprite *dis, World &world)
{
    dis->get_owner()->remove_component<Sprite>();
    dis->get_owner()->remove_component<AnimatedSprite>();

    auto &dissprs = dis->get_dissprs();
    auto &hs = dis->get_hs();
    auto &vs = dis->get_vs();

    for (int i = 0; i < dissprs.size(); i++)
    {
        if (dissprs[i].has_value() && world.components_exist_of_type<Player>())
        {
            auto disspr = dissprs[i].value();
            auto h = hs[i];
            auto v = vs[i];
            if (h.has_value() && v.has_value())
            {
                auto [x1, x2] = h.value();
                auto [y1, y2] = v.value();
                auto dis_tr = dis->get_owner_component<Transform>();
                auto pla_tr = world.get_components<Player>().begin()->second->get_owner_component<Transform>();
                if (pla_tr->get_position().x > dis_tr->get_position().x + x1 &&
                    pla_tr->get_position().x < dis_tr->get_position().x + x2 &&
                    pla_tr->get_position().y > dis_tr->get_position().y + y1 &&
                    pla_tr->get_position().y < dis_tr->get_position().y + y2)
                {
                    if (std::holds_alternative<Sprite *>(disspr))
                    {
                        dis->get_owner()->add_component(std::get<Sprite *>(disspr));
                    }
                    else
                    {
                        dis->get_owner()->add_component(std::get<AnimatedSprite *>(disspr));
                    }
                    break;
                }
            }
            else
            {
                if (std::holds_alternative<Sprite *>(disspr))
                {
                    dis->get_owner()->add_component(std::get<Sprite *>(disspr));
                }
                else
                {
                    dis->get_owner()->add_component(std::get<AnimatedSprite *>(disspr));
                }
                break;
            }
        }
    }
}