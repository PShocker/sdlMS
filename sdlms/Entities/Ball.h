#pragma once

#include "Components/Components.h"
#include <vector>
#include "entt/entt.hpp"

entt::entity load_ball(AnimatedSpriteWarp *asprw, Transform *tran,
                       entt::entity owner, AnimatedSpriteWarp *hit = nullptr, int rotate = 0);

std::vector<entt::entity> load_ball(unsigned char num = 1);

std::vector<entt::entity> load_ball(AnimatedSpriteWarp *asprw, entt::entity owner,
                                    unsigned char num = 1, AnimatedSpriteWarp *hit = nullptr, int rotate = 0);
