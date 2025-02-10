#include "Damage.h"
#include "Sprite.h"

#include "wz/Property.hpp"
#include "Resources/Wz.h"

void Damage::init()
{
    {
        auto node = Wz::Effect->get_root()->find_from_path(u"BasicEff.img/NoRed1");
        for (int i = 0; i < 10; i++)
        {
            red[i] = SpriteWarp::load(node->get_child(std::to_string(i)), 255, false);
        }
    }
    {
        auto node = Wz::Effect->get_root()->find_from_path(u"BasicEff.img/NoViolet1");
        for (int i = 0; i < 10; i++)
        {
            violet[i] = SpriteWarp::load(node->get_child(std::to_string(i)), 255, false);
        }
    }
    {
        auto node = Wz::Effect->get_root()->find_from_path(u"BasicEff.img/NoCri1");
        for (int i = 0; i < 10; i++)
        {
            cri[i] = SpriteWarp::load(node->get_child(std::to_string(i)), 255, false);
        }
    }
    {
        auto node = Wz::Effect->get_root()->find_from_path(u"BasicEff.img/NoBlue1");
        for (int i = 0; i < 10; i++)
        {
            blue[i] = SpriteWarp::load(node->get_child(std::to_string(i)), 255, false);
        }
    }
}
