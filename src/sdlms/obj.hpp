#ifndef HEADER_SDLMS_OBJ
#define HEADER_SDLMS_OBJ

#include <SDL2/SDL.h>
#include <vector>
#include <string>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"
#include "sdlms/animatedsprite.hpp"

class Obj : public AnimatedSprite
{
public:
    Obj(AnimatedSprite animatedsprite,
        int layer, int z,
        std::u16string url);

public:
    int _layer;
    int _z;

    std::u16string _url;

private:
    Camera *_camera;
    Graphics *_graphics;
};
#endif