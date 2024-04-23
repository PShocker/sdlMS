#ifndef HEADER_SDLMS_BACKGRD
#define HEADER_SDLMS_BACKGRD

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <variant>

#include "sdlms/animatedsprite.hpp"
#include "sdlms/sprite.hpp"
#include "sdlms/dynamicsprite.hpp"
#include "sdlms/camera.hpp"

class BackGrd : public DynamicSprite
{
public:
    BackGrd(std::variant<Sprite, AnimatedSprite> dynamicsprite,
            int id, int type,
            int front,
            int rx, int ry,
            int cx, int cy,
            int ani,
            std::u16string url);
    void update(int elapsedTime);
    static void updates(std::vector<BackGrd> &backgrd, int elapsedTime);
    static void drawbackgrounds(std::vector<BackGrd> &backgrd);
    std::pair<std::vector<BackGrd>, std::vector<BackGrd>> load_backgrd(int mapId);
    std::pair<std::vector<BackGrd>, std::vector<BackGrd>> load_backgrd(wz::Node *node);

public:
    int _id;
    int _type;
    int _front;
    int _rx;
    int _ry;
    int _cx;
    int _cy;
    int _ani;
    float _position_offset_x;
    float _position_offset_y;
    std::u16string _url;
    std::vector<DynamicSprite> _backgrds;

private:
    enum Type
    {
        NORMAL,
        HTILED,
        VTILED,
        TILED,
        HMOVEA,
        VMOVEA,
        HMOVEB,
        VMOVEB
    };

private:
    Camera *_camera;
};
#endif