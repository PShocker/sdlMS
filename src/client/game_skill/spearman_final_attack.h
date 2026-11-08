#pragma once

#include <cstdint>

namespace sdlms {

class GameCharacter;

// Spearman 1301000/1301001：攻击后按 WZ prop 注册一次 1301006/1301007。
void RegisterSpearmanFinalAttack(GameCharacter& g_character,
                                 uint64_t attack_delay);

// 处理 Spearman 到期的 Final Attack。
void UpdateSpearmanFinalAttack();

}  // namespace sdlms
