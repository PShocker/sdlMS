#include "C_Cursor.h"

#include "String.h"

#include "Components/Sprite.h"
#include "Components/Transform.h"
#include "Components/RelativeTransform.h"
#include "Components/RandomInput.h"
#include "Components/LimitTransform.h"
#include "Components/Line.h"
#include "Components/Physic/Normal.h"
#include "Resource/Wz.h"

C_Cursor::C_Cursor(wz::Node *node, int id, int rx0, int rx1, World *world)
{
    
}

// 测试
C_Cursor::C_Cursor(World *world)
{
    auto node_cursor = world->get_resource<Wz>().UI->get_root()->find_from_path(u"Basic.img/Cursor");

		for (auto &[name, val] : node_cursor->get_children())
		{
			if (name != u"info")
			{
				auto aspr = new AnimatedSprite(val[0]);
				auto id = std::stoi(std::string{name.begin(), name.end()});
				aspr_map[static_cast<State>(id)] = aspr;
                world->add_component(aspr);
			}
		}

    act = State::IDLE;

    /* add_component(aspr_map[state]);

    Transform *tr = new Transform();
    add_component(tr);
    world->add_component(tr, 3000000); */
}

/* void C_Cursor::switch_act(State s)
{
    if (s != state)
    {
        if (aspr_map.contains(s))
        //if (aspr_map1.contains(""))
        {
            auto aspr = aspr_map[s];
            aspr->set_anim_index(0);
            aspr->set_anim_time(0);
            state = s;
            add_component(aspr);
        }
    }
}

void C_Cursor::start_animate()
{
    get_component<AnimatedSprite>()->set_animate(true);
}

void C_Cursor::stop_animate() 
{
    get_component<AnimatedSprite>()->set_animate(false);
}
*/

C_Cursor::~C_Cursor()
{
    /* auto world = World::get_world();

    for (auto &[key, val] : aspr_map)
    {
        auto aspr = val;
        world->destroy_component(aspr, false);
        delete aspr;
    }

    for (auto &[key, val] : get_entity<String>())
    {
        auto t = val->get_component<Transform>();
        auto rtr = val->get_component<RelativeTransform>();
        world->destroy_component(t, false);
        world->destroy_component(rtr, false);
        delete t;
        delete rtr;
        delete val;
    }

    for (auto &[key, val] : get_entity<NameTag>())
    {
        auto t = val->get_component<Transform>();
        auto rtr = val->get_component<RelativeTransform>();
        world->destroy_component(t, false);
        world->destroy_component(rtr, false);
        delete t;
        delete rtr;
        delete val;
    }

    auto t = get_component<Transform>();
    world->destroy_component(t, false);
    delete t;

    auto nor = get_component<Normal>();
    if (nor != nullptr)
    {
        world->destroy_component(nor, false);
        delete nor;
    }

    auto ran = get_component<RandomInput>();
    if (ran != nullptr)
    {
        world->destroy_component(ran, false);
        delete ran;
    }

    auto ltr = get_component<LimitTransform>();
    if (ltr != nullptr)
    {
        world->destroy_component(ltr, false);
        delete ltr;
    } */
}
