#pragma once

#include <cstdint>

namespace sdlms {

struct CheckMobs;
struct GameMob;

// 3200001 Amazon's Judgement: critical hits slow the target.
void ApplyCrossbowAmazonJudgement(GameMob& mob, uint64_t delay);

// 3201000 Final Attack: Crossbow. Only called after projectile attacks.
void TriggerCrossbowFinalAttack(CheckMobs& cm, uint64_t delay);

}  // namespace sdlms
