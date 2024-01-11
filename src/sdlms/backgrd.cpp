#include "sdlms/backgrd.hpp"

BackGrd::BackGrd(std::vector<SDL_Texture *> texture, std::vector<SDL_Rect *> rect, std::vector<int> delay,
                 std::vector<int> format, int type, int front,
                 int frameIndex, int frameSize,
                 std::u16string url) : _texture(texture), _rect(rect),
                                       _delay(delay), _format(format),
                                       _type(type), _front(front),
                                       _frameIndex(frameIndex),
                                       _frameSize(frameSize),
                                       _frameTime(0),
                                       _url(url)
{
    _camera = Camera::current();
    _graphics=Graphics::current();
}

void BackGrd::draw()
{
    SDL_Rect rect{_rect[_frameIndex]->x, _rect[_frameIndex]->y, _rect[_frameIndex]->w, _rect[_frameIndex]->h};
    rect.x -= _camera->viewport.x;
    _graphics->blitSurface(_texture[_frameIndex], NULL, &rect);
}