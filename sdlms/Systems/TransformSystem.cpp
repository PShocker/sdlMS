#include "TransformSystem.h"

#include "Entities/Border.h"
#include "Components/Physic/Normal.h"

void TransformSystem::run(World &world)
{
    if (world.components_exist_of_type<LimitTransform>())
    {
        for (auto &[index, ltr] : world.get_components<LimitTransform>())
        {
            update_limit_tr(ltr, world);
        }
    }
    if (world.components_exist_of_type<RelativeTransform>())
    {
        for (auto &[index, rtr] : world.get_components<RelativeTransform>())
        {
            update_relative_tr(index, rtr, world);
        }
    }
}

void TransformSystem::update_relative_tr(int index, RelativeTransform *rtr, World &world)
{
    auto ptr = rtr->get_tr();
    auto tr = rtr->get_owner_component<Transform>();
    world.destroy_component(tr, false);
    tr->set_position(ptr->get_position() + rtr->get_position());
    world.add_component(tr, index + ptr->get_id());
}

void TransformSystem::update_limit_tr(LimitTransform *ltr, World &world)
{
    auto &tr = ltr->get_tr();
    if (ltr->get_h().has_value())
    {
        // 如果水平方向存在范围
        auto left = ltr->get_h().value().x;
        auto right = ltr->get_h().value().y;
        if (tr->get_position().x != std::clamp(tr->get_position().x, left, right))
        {
            tr->set_x(std::clamp(tr->get_position().x, left, right));
            tr->get_owner_component<Normal>()->hspeed = 0;
        }
    }
    else
    { // 如果不存在水平方向范围,则与地图空气墙碰撞
        if (world.entity_exist_of_type<Border>())
        {
            auto border = world.get_entitys<Border>().find(0)->second;
            auto left = border->get_left();
            auto right = border->get_right();
            if (left.has_value() && tr->get_position().x <= left.value() + 10)
            {
                // 水平方向撞墙
                tr->set_x(left.value() + 10);
                tr->get_owner_component<Normal>()->hspeed = 0;
            }
            else if (right.has_value() && tr->get_position().x >= right.value() - 10)
            {
                // 水平方向撞墙
                tr->set_x(right.value() - 10);
                tr->get_owner_component<Normal>()->hspeed = 0;
            }
        }
    }
    // 判断竖直方向范围
    if (ltr->get_v().has_value())
    {
        auto top = ltr->get_v().value().x;
        auto bottom = ltr->get_v().value().y;
        // 如果竖直方向存在范围
        if (tr->get_position().y != std::clamp(tr->get_position().y, top, bottom))
        {
            tr->set_y(std::clamp(tr->get_position().y, top, bottom));
            tr->get_owner_component<Normal>()->vspeed = 0;
        }
    }
    else
    { // 如果不存在竖直方向范围,则与地图空气墙碰撞
        if (world.entity_exist_of_type<Border>())
        {
            auto border = world.get_entitys<Border>().find(0)->second;
            auto top = border->get_top();
            auto bottom = border->get_bottom();
            if (top.has_value() && tr->get_position().y < top.value())
            {
                // 竖直方向撞墙
                tr->set_y(top.value());
                tr->get_owner_component<Normal>()->vspeed = 0;
            }
            else if (bottom.has_value() && tr->get_position().y > bottom.value())
            {
                // 竖直方向撞墙
                tr->set_y(bottom.value());
                tr->get_owner_component<Normal>()->vspeed = 0;
            }
        }
    }
}
