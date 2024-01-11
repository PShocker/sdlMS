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
}