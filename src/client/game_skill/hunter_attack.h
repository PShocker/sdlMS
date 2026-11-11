#pragma once

#include <cstdint>

namespace sdlms {

struct CheckMobs;
struct GameMob;

// 3100001 Amazon's Judgement: critical hits slow the target.
void ApplyAmazonJudgement(GameMob& mob, uint64_t delay);

// 3101000 Final Attack: Bow. Checked after a bow attack payload is built.
void TriggerBowFinalAttack(CheckMobs& cm, uint64_t delay);

}  // namespace sdlms
