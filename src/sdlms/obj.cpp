#include "sdlms/obj.hpp"

Obj::Obj(std::vector<SDL_Texture *> texture, std::vector<SDL_Rect *> rect,
         std::vector<int> delay,
         std::vector<int> format, int layer,
         int z, std::u16string url, int frameSize) : _texture(texture), _rect(rect),
                                                     _delay(delay), _format(format),
                                                     _layer(layer), _z(z), _frameIndex(0),
                                                     _frameTime(0),
                                                     _url(url), _frameSize(frameSize)
{
    _camera = Camera::current();
}

void Obj::update(int elapsedTime)
{
    if (_frameSize == 1)
    {
        return;
    }
    else
    {
        _frameTime += elapsedTime;
        if (_frameTime >= _delay[_frameIndex])
        {
            if (_frameIndex == _frameSize - 1)
            {
                _frameIndex = 0;
            }
            else
            {
                _frameIndex += 1;
            }
            // 切换下一帧
            _frameTime = 0;
        }
    }
}
void Obj::draw()
{
    SDL_Rect rect{_rect[_frameIndex]->x, _rect[_frameIndex]->y, _rect[_frameIndex]->w, _rect[_frameIndex]->h};
    rect.x -= _camera->viewport.x;
    Graphics::current()->blitSurface(_texture[_frameIndex], NULL, &rect);
}
