#pragma once

namespace sdlms {

struct CheckMobs;

// MP Eater：魔法攻击命中后按 WZ prop 概率吸取目标 MP。
// 2100000 为 FP Wizard，2200000 为 IL Wizard。
void TriggerMpEater(CheckMobs& cm);

}  // namespace sdlms
