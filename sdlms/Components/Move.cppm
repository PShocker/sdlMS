module;

#include <optional>
#include <SDL3/SDL.h>

export module components:move;

export struct Move
{
    float hspeed = 0.0;
    std::optional<SDL_FPoint> hspeed_limit = SDL_FPoint{-125.0f, 125.0f};
    float vspeed = 0.0;
    std::optional<SDL_FPoint> vspeed_limit = SDL_FPoint{-5000.0f, 670.0f};
    float hforce = 0.0;
    float vforce = 0.0;
    float hacc = 0.0;
    float vacc = 0.0;

    std::optional<float> rx0;
    std::optional<float> rx1;

    enum AREA : uint8_t
    {
        LAND,
        SWIM,
    };

    uint8_t area = LAND;

    Move() = default;
};
