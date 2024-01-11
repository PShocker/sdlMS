#include "sdlms/sprite.hpp"


Sprite::Sprite(SDL_Texture *texture, SDL_Rect *rect, int format) : _texture(texture),
                                                                   _rect(rect),
                                                                   _format(format)
{
    _camera = Camera::current();
    _graphics = Graphics::current();
}
