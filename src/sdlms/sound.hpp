#ifndef HEADER_SDLMS_SOUND
#define HEADER_SDLMS_SOUND

#include <cstdint>
#include <vector>

#include <SDL2/SDL.h>
#include "util/currenton.hpp"

class Sound : public Currenton<Sound>
{
public:
    Sound();
    int load(std::vector<uint8_t> data);
};

#endif
