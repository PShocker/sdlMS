#pragma once

#include <cstdint>

namespace sdlms {

class GameCharacter;

// 095 TryRegisterFinalAttack：攻击命中后按 WZ prop 注册一次 Final Attack。
void RegisterFinalAttack(GameCharacter& g_character, uint64_t attack_delay);

// 处理到期的 Final Attack。
void UpdateFinalAttack();

}  // namespace sdlms
