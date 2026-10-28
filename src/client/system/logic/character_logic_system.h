#ifndef SDLMS_SRC_CLIENT_SYSTEM_LOGIC_CHARACTER_LOGIC_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_LOGIC_CHARACTER_LOGIC_SYSTEM_H_

#include <SDL3/SDL_rect.h>
#include "src/client/game/game_character.h"
#include "src/client/game/game_input.h"
#include "src/client/game/game_mob.h"
#include "src/client/game/game_reactor.h"
#include "src/client/game/game_triangle.h"
#include "src/common/flatbuffers/client.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>
#include <vector>
namespace sdlms {



struct CheckMobs {
  struct Mobs {
    GameMob mob;
    float x;
    float y;
    std::vector<int64_t> hits;
    uint64_t delay = 0;
  };
  std::vector<Mobs> data;
};

struct CheckReactors {
  struct Reactors {
    GameReactor r;
    float x;
    float y;
  };
  std::vector<Reactors> data;
};

class CharacterLogicSystem {
private:
  static CheckReactors RunReactorCheck(GameCharacter &g, SDL_FRect g_r);

  static void RunClimbAction(GameCharacter &g_character);
  static void RunFaceAnimate(GameCharacter &g_character);
  static bool RunFlip(GameCharacter &g_character);
  static void RunPick(GameCharacter &g_character);
  static bool RunWalk(GameCharacter &g_character);
  static bool RunFall(GameCharacter &g_character);
  static bool RunJump(GameCharacter &g_character);
  static bool RunProne(GameCharacter &g_character);
  static bool RunClimb(GameCharacter &g_character);
  static bool RunClimbing(GameCharacter &g_character);
  static bool RunSit(GameCharacter &g_character);
  static bool RunSitting(GameCharacter &g_character);

  static bool RunAttack(GameCharacter &g_character);

  static bool RunSkill(GameCharacter &g_character);
  static bool RunPortal(GameCharacter &g_character);
  static bool RunFace(GameCharacter &g_character);
  static void RunTomb(GameCharacter &g_character);

  static void RunItem(GameCharacter &g_character);

  static void RunStateMachine();
  static void RunColor();
  enum class PosType {
    kLand,
    kSwim,
    kFly,
  };
  static PosType LoadPosType(GameCharacter &g_character);

  static void RunOthersStateMachine();
  static void RunOthersMv();
  static void RunOthers();
  static void RunNetworkSync();
  static void RunNetworkSyncAction();
  static void RunNetworkSyncPos();
  static void RunNetworkSyncFlip();
  static void RunNetworkSyncFace();
  static void RunNetworkSyncSelf();
  static void RunNetworkSyncHp();
  static void RunNetworkSyncMaxHp();
  static void RunNetworkSyncState();

  static void RunAnimateChair(GameCharacter &g_character);

public:
  static inline fbs::ClientCharacterStateT ccs_;
  static inline fbs::ClientCharacterPickT ccp_;
  static inline fbs::ClientCharacterT cct_;

  static bool RunHarvest(GameCharacter &g_character); //or npc talk

  static void RunBeingHit(float x, uint64_t num);

  static void RunSitChair(GameCharacter &g_character,
                            const std::u16string &id);
  static void RunUnsitChair(GameCharacter &g_character);

  static bool RunSkill(GameCharacter &g_character, const std::u16string &id);

  static std::vector<uint64_t> RunBuffCheck(GameCharacter &g_character,
                                              SDL_FRect g_r);
  static CheckMobs RunAttackCheck(SDL_FPoint pos, SDL_FRect g_r, int num);
  static CheckMobs RunAttackCheck(GameCharacter &g_character, SDL_FRect g_r,
                                     int num);
  static CheckMobs RunAttackCheck(GameCharacter &g_character,
                                     SDL_FRect g_r);
  static CheckMobs RunAttackCheck(GameCharacter &g_character,
                                     GameTriangle tri, int num);
  static CheckMobs RunAttackCheck(GameCharacter &g_character,
                                     GameTriangle tri);
  static bool RunAnimate(GameCharacter &g_character);
  static void RunNetworkDieSync(GameCharacter &g_character);
  static bool RunAction(GameCharacter &g_character,
                         const std::u16string &action);
  static bool RunFaceAction(GameCharacter &g_character,
                              const std::u16string &action);
  static void RunDieAction(GameCharacter &g_character);
  static void RunStandAction(GameCharacter &g_character);
  static void RunWalkAction(GameCharacter &g_character);
  static bool RunAttackAction(GameCharacter &g_character, bool shoot);

  static void LoadSfx(GameCharacter &g_character);
  static float LoadAttackSpeed(GameCharacter &g_character);

  static std::u16string LoadMorphType(GameCharacter &g_character);
  static SDL_FRect LoadRect(GameCharacter &g_character);
  enum class ActionEnum {
    kStand,
    kAlert,
    kWalk,
    kProne,
    kJump,
    kClimb,
    kSit,
    kAttack,
    kSkill,
    kDead,
  };
  static ActionEnum LoadActionType(GameCharacter &g_character);
  static bool Run();
  static inline std::flat_set<std::string> character_action_input_;
  static inline std::flat_set<std::string> character_skill_input_;
  static inline std::flat_set<std::string> character_face_input_;
  static inline std::flat_set<GameInput> character_item_input_;

  static inline int32_t self_fh_; // foothold
  static inline int32_t self_lr_; // ladderrope

  static inline float self_hforce_ = 0.0;
  static inline float self_vforce_ = 0.0;

  static inline float self_hspeed_ = 0.0;
  static inline float self_vspeed_ = 0.0;

  static inline float self_hspeed_min_ = -125.0f;
  static inline float self_hspeed_max_ = 125.0f;

  static inline float self_vspeed_min_ = -5550.0f;
  static inline float self_vspeed_max_ = 670.0f;

  static inline uint64_t self_alert_cooldown_;
  static inline uint64_t self_ladderrope_cooldown_;
  static inline uint64_t self_foothold_cooldown_;
  static inline uint64_t self_sit_cooldown_;
  static inline uint64_t self_face_cooldown_;
  static inline uint64_t self_portal_cooldown_;
  static inline uint64_t self_flip_cooldown_;
  static inline uint64_t self_attack_cooldown_;
  static inline uint64_t self_invincible_cooldown_;
  static inline uint64_t self_two_jump_cooldown_;

  static inline uint64_t self_climb_animate_cooldown_; // 绳子放技能,动作时间

  static inline float self_fall_min_;
  static inline int32_t self_fall_fh_;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_LOGIC_CHARACTER_LOGIC_SYSTEM_H_
