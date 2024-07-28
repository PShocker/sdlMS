module;

module components;

import :playerskill;

Attack::Attack(PlayerSkill *pski)
{
    auto lt = pski->ski->infos[pski->level]->lt;
    auto rb = pski->ski->infos[pski->level]->rb;
    rect.x = lt.x;
    rect.y = lt.y;
    rect.w = rb.x - lt.x;
    rect.h = rb.y - lt.y;
    hit = pski->ski->hits[0];
}