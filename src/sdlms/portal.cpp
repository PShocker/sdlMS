#include "sdlms/portal.hpp"

Portal::Portal(std::variant<Sprite, AnimatedSprite> dynamicsprite, int type, std::u16string url) : DynamicSprite(dynamicsprite),
                                                                                                   _type(type),
                                                                                                   _url(url)
{
    _camera = Camera::current();
}

void Portal::draw()
{
    // 只绘制GAME类型的传送门
    if (_type == Type::GAME)
    {
        DynamicSprite::draw();
    }
}
void Portal::event(const SDL_Event &event)
{
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && _type == Type::GAME)
    {
        SDL_FPoint p{(float)event.button.x + _camera->viewport.x, (float)event.button.y + _camera->viewport.y};
        auto rf = rect();
        if (SDL_PointInFRect(&p, &rf))
        {
            printf("123");
        }
    }
}
