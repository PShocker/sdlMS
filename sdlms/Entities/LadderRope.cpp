#include "LadderRope.h"
#include "Components/CrawlLine.h"
#include "Components/Physic/Normal.h"

LadderRope::LadderRope(wz::Node *node, int id, World *world) : id(id)
{
    l = dynamic_cast<wz::Property<int> *>(node->get_child(u"l"))->get();
    uf = dynamic_cast<wz::Property<int> *>(node->get_child(u"uf"))->get();
    page = dynamic_cast<wz::Property<int> *>(node->get_child(u"page"))->get();

    auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
    auto y1 = dynamic_cast<wz::Property<int> *>(node->get_child(u"y1"))->get();
    auto y2 = dynamic_cast<wz::Property<int> *>(node->get_child(u"y2"))->get();

    auto *c = new CrawlLine{{(float)x, (float)y1}, {(float)x, (float)y2}};
    add_component(c);
}

LadderRope::~LadderRope()
{
    // 摧毁lp,需要删除lp关联到ent的map
    auto world = World::get_world();
    if (world->components_exist_of_type<Normal>())
    {
        // 删除所有nor的ent关联到lp的map
        for (auto &[key, val] : world->get_components<Normal>())
        {
            auto nor = val;
            if (nor->get_owner()->get_entity<LadderRope>() == this)
            {
                nor->get_owner()->remove_entity<LadderRope>();
                nor->type = Normal::Air;
            }
        }
    }
    auto c = get_component<CrawlLine>();
    delete c;
}
