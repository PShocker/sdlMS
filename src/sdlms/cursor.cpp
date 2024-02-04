#include "sdlms/cursor.hpp"

Cursor::Cursor()
{
    _camera = Camera::current();

    _input = Input::current();
    _input->event(std::bind(&event, this, std::placeholders::_1));
}

void Cursor::event(SDL_Event &event)
{
    if (_input->isMouseHeld(SDL_BUTTON_LEFT))
    {
        if (_event != Event::PRESSED)
        {
            _event = Event::PRESSED;
            SDL_SetCursor(_c.at(_event).at(0));
        }
    }
    else if (_input->wasMouseReleased(SDL_BUTTON_LEFT))
    {
        if (_event != Event::NORMAL)
        {
            _event = Event::NORMAL;
            SDL_SetCursor(_c.at(_event).at(0));
        }
    }
}

void Cursor::load()
{
    for (auto &it : _s)
    {
        std::vector<SDL_Cursor *> v;
        auto dynamicsprite = it.second._dynamicsprite;
        if (std::holds_alternative<Sprite>(dynamicsprite))
        {
            auto sprite = std::get<Sprite>(dynamicsprite);
            SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(sprite._raw_data.data(), sprite.rect().w,
                                                                      sprite.rect().h, 16, sprite.rect().w * 2,
                                                                      SDL_PIXELFORMAT_ARGB4444);
            SDL_Cursor *cursor = SDL_CreateColorCursor(surface, 0, 0);
            SDL_FreeSurface(surface);
            v.push_back(cursor);
        }
        else if (std::holds_alternative<AnimatedSprite>(dynamicsprite))
        {
            auto animatedsprite = std::get<AnimatedSprite>(dynamicsprite);
            for (size_t i = 0; i < animatedsprite._frameSize; i++)
            {
                auto sprite = animatedsprite.sprite(i);
                SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(sprite._raw_data.data(), sprite.rect().w,
                                                                          sprite.rect().h, 16, sprite.rect().w * 2,
                                                                          SDL_PIXELFORMAT_ARGB4444);
                SDL_Cursor *cursor = SDL_CreateColorCursor(surface, 0, 0);
                SDL_FreeSurface(surface);
                v.push_back(cursor);
            }
        }
        _c.emplace(it.first, v);
    }
}
