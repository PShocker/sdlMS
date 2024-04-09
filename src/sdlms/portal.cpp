#include "sdlms/portal.hpp"
#include "portal.hpp"

Portal::Portal(std::variant<Sprite, AnimatedSprite> dynamicsprite,
               int type, int tm,
               std::u16string url) : DynamicSprite(dynamicsprite),
                                     _type(type), _tm(tm),
                                     _url(url)
{
    _camera = Camera::current();

    _input = Input::current();
    // _input->event(std::bind(&event, this, std::placeholders::_1));
}

void Portal::draws(std::vector<Portal> portal)
{
    // 只绘制GAME类型的传送门
    for (auto &it : portal)
    {
        if (it._type == Type::GAME)
        {
            it.draw();
        }
    }
}

void Portal::updates(std::vector<Portal> &portals, int elapsedTime)
{
    for (auto &it : portals)
    {
        it.update(elapsedTime);
    }
}
void Portal::event(const SDL_Event &event)
{
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
    {
        if (_type == Type::GAME)
        {
            SDL_FPoint p{(float)event.button.x + _camera->_viewport.x, (float)event.button.y + _camera->_viewport.y};
            auto rf = rect();
            if (SDL_PointInFRect(&p, &rf))
            {
                printf("123Shocker");
            }
        }
    }
}
