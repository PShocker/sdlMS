#include "sdlms/obj.hpp"
#include "obj.hpp"

Obj::Obj(int id,
         AnimatedSprite animatedsprite,
         int layer,
         int z, std::u16string url) : _id(id),
                                      AnimatedSprite(animatedsprite),
                                      _layer(layer), _z(z),
                                      _url(url)
{
}

void Obj::draws(std::vector<Obj> &objs)
{
    for (auto &it : objs)
    {
        it.draw();
    }
}

void Obj::updates(std::vector<Obj> &objs, int elapsedTime)
{
    for (auto &it : objs)
    {
        // 更新帧状态
        it.update(elapsedTime);
    }
}
