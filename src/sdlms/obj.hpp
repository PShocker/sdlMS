#ifndef HEADER_SDLMS_OBJ
#define HEADER_SDLMS_OBJ

#include <SDL2/SDL.h>
#include <vector>
#include <string>

#include "sdlms/animatedsprite.hpp"

class Obj : public AnimatedSprite
{
public:
    Obj(int id,
        AnimatedSprite animatedsprite,
        int layer, int z,
        std::u16string url);
    static void draws(std::vector<Obj> &objs);
    static void updates(std::vector<Obj> &objs, int elapsedTime);
    std::array<std::vector<Obj>, 8> load_obj(int mapId);
    std::vector<Obj> load_obj(wz::Node *node, int i);

public:
    int _id;
    int _layer;
    int _z;

    std::u16string _url;
};
#endif