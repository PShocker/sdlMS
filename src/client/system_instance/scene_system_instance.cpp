#include "scene_system_instance.h"
#include "cash_shop_system_instance.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_drop.h"
#include "src/client/game/game_mob.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/backgrnd_game_instance.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/ladderrope_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/obj_game_instance.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/game_instance/reactor_game_instance.h"
#include "src/client/game_instance/seat_game_instance.h"
#include "src/client/game_instance/tile_game_instance.h"
#include "src/client/game_instance/tooltip_game_instance.h"
#include "src/client/game_instance/trap_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/logic/audio_logic_system.h"
#include "src/client/system/logic/backgrnd_logic_system.h"
#include "src/client/system/logic/ball_logic_system.h"
#include "src/client/system/logic/camera_logic_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/chatballoon_logic_system.h"
#include "src/client/system/logic/cursor_logic_system.h"
#include "src/client/system/logic/drop_logic_system.h"
#include "src/client/system/logic/effect_logic_system.h"
#include "src/client/system/logic/item_logic_system.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/system/logic/npc_logic_system.h"
#include "src/client/system/logic/obj_logic_system.h"
#include "src/client/system/logic/portal_logic_system.h"
#include "src/client/system/logic/reactor_logic_system.h"
#include "src/client/system/logic/skill_logic_system.h"
#include "src/client/system/logic/trap_logic_system.h"
#include "src/client/system/render/backgrnd_render_system.h"
#include "src/client/system/render/ball_render_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/drop_render_system.h"
#include "src/client/system/render/effect_render_system.h"
#include "src/client/system/render/mob_render_system.h"
#include "src/client/system/render/npc_render_system.h"
#include "src/client/system/render/obj_render_system.h"
#include "src/client/system/render/portal_render_system.h"
#include "src/client/system/render/reactor_render_system.h"
#include "src/client/system/render/tile_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/buff_ui_system.h"
#include "src/client/system/ui/gain_log_ui_system.h"
#include "src/client/system/ui/minimap_ui_system.h"
#include "src/client/system/ui/popup_tip_ui_system.h"
#include "src/client/system/ui/quest_alarm_ui_system.h"
#include "src/client/system/ui/quick_slot_ui_system.h"
#include "src/client/system/ui/statusbar_ui_system.h"
#include "src/client/system_instance/fade_system_instance.h"
#include "src/client/system_instance/login_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"
#include <array>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

bool SceneSystemInstance::RenderGame() {
  std::array<std::vector<GameCharacter *>, 8> character_array;
  for (auto &other_data :
       CharacterGameInstance::others_ | std::views::values) {
    auto &g_character = other_data.g_character;
    character_array[g_character.page].push_back(&g_character);
  }
  auto &self = CharacterGameInstance::self_;
  character_array[self.page].push_back(&self);

  std::array<std::vector<GameMob *>, 8> mob_array;
  for (auto &mob_data : MobGameInstance::data_ | std::views::values) {
    auto &mob = mob_data.mob;
    mob_array[mob.page].push_back(&mob);
  }

  std::array<std::vector<GameDrop *>, 8> drop_array;
  for (auto &drop : DropGameInstance::data_ | std::views::values) {
    drop_array[drop.page].push_back(&drop);
  }
  //   后景
  for (auto &b_backgrnd : BackgrndGameInstance::back_ | std::views::values) {
    BackgrndRenderSystem::Render(b_backgrnd);
  }
  for (uint8_t i = 0; i < 8; i++) {
    for (auto &obj : ObjGameInstance::data_[i] | std::views::values) {
      ObjRenderSystem::Render(obj);
    }
    for (auto &trap : TrapGameInstance::data_[i] | std::views::values) {
      ObjRenderSystem::Render(trap);
    }
    for (auto &tile : TileGameInstance::data_[i] | std::views::values) {
      TileRenderSystem::Render(tile);
    }
    for (auto &reactor : ReactorGameInstance::data_[i]) {
      ReactorRenderSystem::Render(reactor);
    }
    for (auto &effect : EffectGameInstance::data_[i]) {
      EffectRenderSystem::Render(effect);
    }
    for (auto &npc : NpcGameInstance::data_[i]) {
      NpcRenderSystem::Render(npc);
    }
    for (auto &mob : mob_array[i]) {
      MobRenderSystem::Render(*mob);
    }
    for (auto &ball : BallGameInstance::data_[i]) {
      BallRenderSystem::Render(ball);
    }
    for (auto &character : character_array[i]) {
      CharacterRenderSystem::Render(*character);
    }
    for (auto &drop : drop_array[i]) {
      DropRenderSystem::Render(*drop);
    }
  }
  // 传送门
  for (auto &portal : PortalGameInstance::data_ | std::views::values) {
    PortalRenderSystem::Render(portal);
  }
  // 前景
  for (auto &f_backgrnd : BackgrndGameInstance::front_ | std::views::values) {
    BackgrndRenderSystem::Render(f_backgrnd);
  }
  for (auto &npcs : NpcGameInstance::data_) {
    for (auto &npc : npcs) {
      NpcRenderSystem::RenderChatballoon(npc);
    }
  }
  for (auto &cs : character_array) {
    for (auto &c : cs) {
      CharacterRenderSystem::RenderChatballoon(*c);
    }
  }
  return true;
}

void SceneSystemInstance::EnterPrepare() {
  const auto &pos = SceneSystemInstance::prepare_pos_;
  auto &self = CharacterGameInstance::self_;
  if (pos.has_value()) {
    self.pos = pos.value();
    CharacterLogicSystem::RunAction(self, u"jump");
  }
  CharacterLogicSystem::self_fh_ = 0;
  CharacterLogicSystem::self_lr_ = 0;
  CharacterLogicSystem::self_hspeed_ = 0;
  CharacterLogicSystem::self_vspeed_ = 0;
  CharacterLogicSystem::RunUnsitChair(self);

  self.action_animate = true;
  self.tomb = std::nullopt;
  self.skill = std::nullopt;
}

void SceneSystemInstance::Enter(uint32_t map_id) {
  EnterPrepare();
  SceneSystemInstance::map_id_ = map_id;

  FootholdGameInstance::data_ = FootholdGameInstance::Load(map_id);
  BackgrndGameInstance::Load(map_id);
  LadderropeGameInstance::Load(map_id);
  MobGameInstance::Load(map_id);
  NpcGameInstance::Load(map_id);
  ObjGameInstance::Load(map_id);
  TrapGameInstance::Load(map_id);
  PortalGameInstance::data_ = PortalGameInstance::Load(map_id);
  ReactorGameInstance::Load(map_id);
  TileGameInstance::Load(map_id);
  TooltipGameInstance::Load(map_id);
  SeatGameInstance::Load(map_id);
  MinimapUiSystem::Load();
  CharacterGameInstance::others_.clear();
  EffectGameInstance::Reset();
  CameraGameInstance::Reset();
  StatusbarUiSystem::Reset();
  BallGameInstance::Reset();
  DropGameInstance::Reset();
  auto bgm = MapInfoGameInstance::LoadBgm(map_id);
  AudioGameInstance::LoadBackgrndAudio(bgm);

  System::event_systems_ = {
      QuestAlarmUiSystem::Event, MinimapUiSystem::Event,
      QuickSlotUiSystem::Event,  StatusbarUiSystem::Event,
      BuffUiSystem::Event,        CursorLogicSystem::Event,
      KeyboardInputSystem::Event,
  };

  System::logic_systems_ = {
      CameraLogicSystem::Run,    BackgrndLogicSystem::Run,
      BallLogicSystem::Run,      EffectLogicSystem::Run,
      CharacterLogicSystem::Run, ChatballoonLogicSystem::Run,
      DropLogicSystem::Run,      MobLogicSystem::Run,
      NpcLogicSystem::Run,       PortalLogicSystem::Run,
      AudioLogicSystem::Run,     ObjLogicSystem::Run,
      CursorLogicSystem::Run,    SkillLogicSystem::Run,
      ReactorLogicSystem::Run,   ItemLogicSystem::Run,
      TrapLogicSystem::Run,
  };
  System::render_systems_ = {
      RenderGame,
      BuffUiSystem::Render,
      StatusbarUiSystem::Render,
      QuickSlotUiSystem::Render,
      PopupTipUiSystem::Render,
      MinimapUiSystem::Render,
      QuestAlarmUiSystem::Render,
      GainLogUiSystem::Render,
      CursorRenderSystem::Render,
  };
  FadeSystemInstance::EnterOut();
  Window::delta_time_ = 0;
}

SDL_FPoint SceneSystemInstance::LoadPreparePos(const std::u16string &pn,
                                                   uint8_t index) {
  SDL_FPoint r;
  GamePortal *por;
  auto portals =
      PortalGameInstance::Load(SceneSystemInstance::prepare_map_id_);
  if (index == 0) {
    // 只需要第一个元素，使用 find 更高效
    auto it = portals.find(pn);
    if (it != portals.end()) {
      por = &it->second;
    }
  } else {
    // 需要第 N 个元素（N>0）
    auto [first, last] = portals.equal_range(pn);
    if (std::distance(first, last) > index) {
      auto it = std::ranges::next(first, index);
      por = &it->second;
    }
  }
  r = por->pos;
  r.y -= 5;
  return r;
}

void SceneSystemInstance::EnterPrepare(uint32_t map_id,
                                          const std::u16string &pn, int i) {
  prepare_map_id_ = map_id;
  prepare_pos_ = LoadPreparePos(pn, i);

  fbs::ClientSceneT client_scene;
  client_scene.fade = false;
  client_scene.map_id = map_id;

  // 记录起始时间
  Window::dt_time_start_ = Window::dt_now_;
  ClientRequest::send_to_host(client_scene);
}

void SceneSystemInstance::EnterFade() {
  auto g = CharacterGameInstance::self_;
  g.pos = prepare_pos_.value();

  CharacterLogicSystem::RunAction(g, u"jump");
  auto c = ServerCharacterInstance::LoadCharactert(g);

  fbs::ClientSceneT client_scene;
  client_scene.fade = true;
  client_scene.map_id = prepare_map_id_;
  client_scene.character = std::make_unique<fbs::CharacterT>(std::move(c));

  ClientRequest::send_to_host(client_scene);
}

void SceneSystemInstance::EnterQuit() {
  LoginSystemInstance::Enter();
  FadeSystemInstance::EnterOut();
  FadeSystemInstance::EnterOut();
}

void SceneSystemInstance::QuitPrepare() {
  fbs::ClientSceneT client_scene;
  client_scene.fade = false;
  client_scene.map_id = 0;
  ClientRequest::send_to_host(client_scene);
  FadeSystemInstance::EnterIn(SceneSystemInstance::EnterQuit);
}

void SceneSystemInstance::EnterCashShop() {
  CashShopSystemInstance::Enter();
  FadeSystemInstance::EnterOut();
  FadeSystemInstance::EnterOut();
}

void SceneSystemInstance::CashShopPrepare() {
  fbs::ClientSceneT client_scene;
  client_scene.fade = false;
  client_scene.map_id = 0;
  ClientRequest::send_to_host(client_scene);
  FadeSystemInstance::EnterIn(SceneSystemInstance::EnterQuit);
}