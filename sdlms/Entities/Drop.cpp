#include "Drop.h"
#include "wz/Property.hpp"
#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

void load_drops(std::vector<std::pair<std::u16string, unsigned int>> &drops, float x, float y, int layer)
{
    float hspeed_min = 0;
    float hspeed_max = 0;
    auto count = World::registry->view<Drop>().size();
    for (int i = 0; i < drops.size(); i++)
    {
        auto &[id, nums] = drops[i];
        auto ent = World::registry->create();

        World::registry->emplace<Transform>(ent, (float)x, (float)y, layer * LAYER_Z + DROP_Z + count + i);

        auto &dro = World::registry->emplace<Drop>(ent);
        dro.id = id;
        dro.nums = nums;

        int texture_w = 0;
        if (id == u"09000000")
        {
            AnimatedSprite::Wrap *asprw;
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
            texture_w = asprw->sprites[0]->texture->w;
            dro.spr = AnimatedSprite(asprw);
        }
        else
        {
            auto node = Item::load(id);
            auto sprw = Sprite::Wrap::load(node->find_from_path(u"info/iconRaw"));
            texture_w = sprw->texture->w;
            dro.spr = Sprite(sprw);
        }

        auto &mv = World::registry->emplace<Move>(ent);
        mv.vspeed = -430;
        mv.hspeed = 0;
        if (i >= 1)
        {
            if (i % 2 == 0)
            {
                hspeed_max += texture_w * 1.2;
                mv.hspeed = hspeed_max;
            }
            else
            {
                hspeed_min -= texture_w * 1.2;
                mv.hspeed = hspeed_min;
            }
        }
    }
    return;
}
