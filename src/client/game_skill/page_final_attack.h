#pragma once

#include <cstdint>

namespace sdlms {

class GameCharacter;

// Page 1201000/1201001：攻击后按 WZ prop 注册一次 1201006/1201007。
void RegisterPageFinalAttack(GameCharacter& g_character, uint64_t attack_delay);

// 处理 Page 到期的 Final Attack。
void UpdatePageFinalAttack();

}  // namespace sdlms
