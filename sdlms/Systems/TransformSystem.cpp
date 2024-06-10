#include "TransformSystem.h"

void TransformSystem::run(World &world)
{
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
    tr->set_x((int)tr->get_position().x);
    tr->set_y((int)tr->get_position().y);
    world.add_component(tr, index + ptr->get_id());
}
