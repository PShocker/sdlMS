#ifndef HEADER_SDLMS_PORTAL
#define HEADER_SDLMS_PORTAL

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <variant>

#include "sdlms/animatedsprite.hpp"
#include "sdlms/sprite.hpp"
#include "sdlms/dynamicsprite.hpp"
#include "sdlms/input.hpp"
#include "sdlms/camera.hpp"

class Portal : public DynamicSprite
{
public:
    Portal(std::variant<Sprite, AnimatedSprite> dynamicsprite,
           int type, int tm,
           std::u16string url);
    static void draws(std::vector<Portal> &portal);
    static void updates(std::vector<Portal> &portals, int elapsedTime);

    void event(const SDL_Event &event);

    std::vector<Portal> load_portal(int mapId);
    std::vector<Portal> load_portal(wz::Node *node);

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
    Input *_input;
};

#endif