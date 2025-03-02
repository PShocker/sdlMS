#pragma once

#include "Components/Components.h"
#include <vector>
#include "entt/entt.hpp"

entt::entity load_ball(AnimatedSpriteWarp *asprw, Transform *tran,
                       entt::entity owner, AnimatedSpriteWarp *hit = nullptr,
                       std::optional<int> rotate = std::nullopt,
                       Skill *ski = nullptr);

std::vector<entt::entity> load_ball(unsigned char num, entt::entity owner, std::optional<int> rotate = std::nullopt,
                                    Skill *ski = nullptr);

std::vector<entt::entity> load_ball(AnimatedSpriteWarp *asprw, entt::entity owner,
                                    unsigned char num = 1,
                                    AnimatedSpriteWarp *hit = nullptr,
                                    std::optional<int> rotate = std::nullopt,
                                    Skill *ski = nullptr);

extern const std::unordered_set<std::u16string> track_no_skill;
