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
    static void draws(int i);
    static void updates(int i, int elapsedTime);

    static void load_obj(int mapId);
    static inline std::array<std::vector<Obj>, 8> _obj;

public:
    int _id;
    int _layer;
    int _z;
    std::u16string _url;
};
#endif