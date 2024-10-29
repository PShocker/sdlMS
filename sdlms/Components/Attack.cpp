module;

module components;

import :skill;
import :afterimage;

AttackWarp::AttackWarp(Skill *ski, int level)
{
    auto lt = ski->ski->infos[ski->level]->lt;
    auto rb = ski->ski->infos[ski->level]->rb;
    rect.x = lt.x;
    rect.y = lt.y;
    rect.w = rb.x - lt.x;
    rect.h = rb.y - lt.y;
    hit = ski->ski->hits[0];
    mobCount = ski->ski->infos[ski->level]->mobCount;
    attackCount = ski->ski->infos[ski->level]->attackCount;
    if (ski->ski->sounds.contains(u"Hit"))
    {
        souw = ski->ski->sounds[u"Hit"];
    }
}

AttackWarp::AttackWarp(AfterImage *aft)
{
    auto lt = aft->info.lt;
    auto rb = aft->info.rb;
    rect.x = lt.x;
    rect.y = lt.y;
    rect.w = rb.x - lt.x;
    rect.h = rb.y - lt.y;
    hit = aft->hits[u"sword1"];
}
