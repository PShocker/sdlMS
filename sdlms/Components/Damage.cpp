#include "Damage.h"
#include "Sprite.h"

#include "wz/Property.hpp"
#include "Resources/Wz.h"
#include "Core/Window.h"

void Damage::init()
{
    {
        auto node = Wz::Effect->get_root()->find_from_path(u"BasicEff.img/NoRed1");
        for (int i = 0; i < 10; i++)
        {
            red[i] = SpriteWarp::load(node->get_child(std::to_string(i)), 255);
        }
    }
    {
        auto node = Wz::Effect->get_root()->find_from_path(u"BasicEff.img/NoViolet1");
        for (int i = 0; i < 10; i++)
        {
            violet[i] = SpriteWarp::load(node->get_child(std::to_string(i)), 255);
        }
    }
    {
        auto node = Wz::Effect->get_root()->find_from_path(u"BasicEff.img/NoCri1");
        for (int i = 0; i < 10; i++)
        {
            cri[i] = SpriteWarp::load(node->get_child(std::to_string(i)), 255);
        }
        cri[10] = SpriteWarp::load(node->get_child(u"effect"), 255);
    }
    {
        auto node = Wz::Effect->get_root()->find_from_path(u"BasicEff.img/NoBlue1");
        for (int i = 0; i < 10; i++)
        {
            blue[i] = SpriteWarp::load(node->get_child(std::to_string(i)), 255);
        }
    }
}

void Damage::push(Damage *dam, int damage, uint8_t type, SDL_FPoint point, unsigned int delay)
{
    if ((dam->damages.size() > 0 &&
         dam->damages.front().alpha <= 200) ||
        dam->damages.size() == 0)
    {
        dam->index = 0;
    }
    int count = dam->index;
    if (count == 0)
    {
        dam->damages.push_front({damage, 255, type, delay, SDL_FPoint{point.x, point.y - (float)(count) * 34}});
    }
    else
    {
        dam->damages.push_back({damage, 255, type, delay, SDL_FPoint{point.x, point.y - (float)(count) * 34}});
    }
    dam->index++;
}