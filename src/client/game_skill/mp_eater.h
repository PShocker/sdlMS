#pragma once

namespace sdlms {

struct CheckMobs;

// 魔法攻击命中后按 2100000/MP Eater 的 WZ 规则吸取目标 MP。
void TriggerMpEater(CheckMobs& cm);

}  // namespace sdlms
