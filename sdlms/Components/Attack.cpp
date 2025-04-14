#include "Attack.h"

Attack::Attack(SDL_FPoint &lt, SDL_FPoint &rb,
               AnimatedSprite::Wrap *hit,
               int mobCount, int attackCount,
               Sound::Wrap *souw, int damage) : hit(hit),
                                              mobCount(mobCount),
                                              attackCount(attackCount),
                                              souw(souw), damage(damage)
{
    rect.x = lt.x;
    rect.y = lt.y;
    rect.w = rb.x - lt.x;
    rect.h = rb.y - lt.y;
}