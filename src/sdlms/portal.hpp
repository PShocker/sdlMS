#ifndef HEADER_SDLMS_PORTAL
#define HEADER_SDLMS_PORTAL

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <variant>

#include "sdlms/animatedsprite.hpp"
#include "sdlms/sprite.hpp"
#include "sdlms/dynamicsprite.hpp"

#include "sdlms/camera.hpp"

class Portal : public DynamicSprite
{
public:
    Portal(std::variant<Sprite, AnimatedSprite> dynamicsprite,
           int type, int tm,
           std::u16string url);
    void draw();

    void event(const SDL_Event &event);

public:
    int _type;
    int _tm;
    std::u16string _url;

public:
    enum Type
    {
        EDITOR,
        GAME
    };

private:
    Camera *_camera;
};

#endif