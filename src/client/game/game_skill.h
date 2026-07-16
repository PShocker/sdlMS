#pragma once

#include <cstdint>
#include <string>
class game_skill {
public:
  std::u16string id;
  int lvl;
  
};

class game_buff_skill : public game_skill {
public:
  uint32_t incHP;
  uint32_t incMP;

  uint32_t incHPMax;
  uint32_t incMPMax;

  uint32_t incAttack;
  uint32_t incMagic;
};

class game_attack_skill : public game_skill {
public:
  uint32_t mpCon;
  uint32_t damage;
  uint32_t mobCount;
  uint32_t attackCount;
};

class game_ball_skill : public game_skill {
public:
  uint32_t mpCon;
  uint32_t damage;
  uint32_t bulletCount;
  int x;
  int y;
  uint32_t mobCount;
  uint32_t bulletConsume;
  uint32_t attackCount;
};