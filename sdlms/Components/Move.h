#pragma once

#include "FootHold.h"
#include "LadderRope.h"
#include <optional>

struct Move
{
    float hspeed = 0.0;
    std::optional<float> hspeed_min = -125.0f;
    std::optional<float> hspeed_max = 125.0f;
    float vspeed = 0.0;
    std::optional<float> vspeed_min = -5000.0f;
    std::optional<float> vspeed_max = 670.0f;

    float hforce = 0.0;
    float vforce = 0.0;
    float hacc = 0.0;
    float vacc = 0.0;

    std::optional<float> rx0 = std::nullopt;
    std::optional<float> rx1 = std::nullopt;

    std::optional<float> ry0 = std::nullopt;
    std::optional<float> ry1 = std::nullopt;

    int page = -1;
    int zmass = 0;

    FootHold *foo = nullptr;
    LadderRope *lr = nullptr;

    Move() = default;
};
