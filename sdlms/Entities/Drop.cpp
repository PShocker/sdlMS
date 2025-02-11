#include "Drop.h"

#include "wz/Property.hpp"
#include <any>
#include "entt/entt.hpp"

#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

void load_drops(std::vector<DropInfo> *drops, float x, float y, int layer)
{
    for (int i = 0; i < drops->size(); i++)
    {
        auto &id = drops->at(i).id;
        auto &nums = drops->at(i).nums;
        AnimatedSpriteWarp *asprw = nullptr;
        if (id == u"09000000")
        {
            // 金币
            if (nums < 49)
            {
                asprw = Drop::mesoicons[Drop::MesoIcon::BRONZE];
            }
            else if (nums < 99)
            {
                asprw = Drop::mesoicons[Drop::MesoIcon::GOLD];
            }
            else if (nums < 999)
            {
                asprw = Drop::mesoicons[Drop::MesoIcon::BUNDLE];
            }
            else
            {
                asprw = Drop::mesoicons[Drop::MesoIcon::BAG];
            }
        }
        else
        {
            // 测试
            // asprw = asprw->load(Wz::Item->get_root()->find_from_path(u"Etc/0400.img/04000000/info/iconRaw"));
            asprw = asprw->load(Wz::Character->get_root()->find_from_path(u"Weapon/01382014.img/info/iconRaw"));
        }
        auto ent = World::registry->create();

        World::registry->emplace<Animated>(ent);
        World::registry->emplace<Transform>(ent, (float)x, (float)y, layer * LAYER_Z + DROP_Z);

        auto &dro = World::registry->emplace<Drop>(ent);
        dro.id = id;
        dro.nums = nums;
        dro.aspr = AnimatedSprite(asprw);

        auto &mv = World::registry->emplace<Move>(ent);
        mv.vspeed = -420;
        mv.hspeed = 0;
    }
    return;
}
