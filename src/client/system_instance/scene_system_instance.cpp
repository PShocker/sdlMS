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

bool SceneSystemInstance::render_game() {
  std::array<std::vector<GameCharacter *>, 8> character_array;
  for (auto &other_data :
       CharacterGameInstance::others | std::views::values) {
    auto &g_character = other_data.g_character;
    character_array[g_character.page].push_back(&g_character);
  }
  auto &self = CharacterGameInstance::self;
  character_array[self.page].push_back(&self);

  std::array<std::vector<GameMob *>, 8> mob_array;
  for (auto &mob_data : MobGameInstance::data | std::views::values) {
    auto &mob = mob_data.mob;
    mob_array[mob.page].push_back(&mob);
  }

  std::array<std::vector<GameDrop *>, 8> drop_array;
  for (auto &drop : DropGameInstance::data | std::views::values) {
    drop_array[drop.page].push_back(&drop);
  }
  //   后景
  for (auto &b_backgrnd : BackgrndGameInstance::back | std::views::values) {
    BackgrndRenderSystem::render(b_backgrnd);
  }
  for (uint8_t i = 0; i < 8; i++) {
    for (auto &obj : ObjGameInstance::data[i] | std::views::values) {
      ObjRenderSystem::render(obj);
    }
    for (auto &trap : TrapGameInstance::data[i] | std::views::values) {
      ObjRenderSystem::render(trap);
    }
    for (auto &tile : TileGameInstance::data[i] | std::views::values) {
      TileRenderSystem::render(tile);
    }
    for (auto &reactor : ReactorGameInstance::data[i]) {
      ReactorRenderSystem::render(reactor);
    }
    for (auto &effect : EffectGameInstance::data[i]) {
      EffectRenderSystem::render(effect);
    }
    for (auto &npc : NpcGameInstance::data[i]) {
      NpcRenderSystem::render(npc);
    }
    for (auto &mob : mob_array[i]) {
      MobRenderSystem::render(*mob);
    }
    for (auto &ball : BallGameInstance::data[i]) {
      BallRenderSystem::render(ball);
    }
    for (auto &character : character_array[i]) {
      CharacterRenderSystem::render(*character);
    }
    for (auto &drop : drop_array[i]) {
      DropRenderSystem::render(*drop);
    }
  }
  // 传送门
  for (auto &portal : PortalGameInstance::data | std::views::values) {
    PortalRenderSystem::render(portal);
  }
  // 前景
  for (auto &f_backgrnd : BackgrndGameInstance::front | std::views::values) {
    BackgrndRenderSystem::render(f_backgrnd);
  }
  for (auto &npcs : NpcGameInstance::data) {
    for (auto &npc : npcs) {
      NpcRenderSystem::render_chatballoon(npc);
    }
  }
  for (auto &cs : character_array) {
    for (auto &c : cs) {
      CharacterRenderSystem::render_chatballoon(*c);
    }
  }
  return true;
}

void SceneSystemInstance::enter_prepare() {
  const auto &pos = SceneSystemInstance::prepare_pos;
  auto &self = CharacterGameInstance::self;
  if (pos.has_value()) {
    self.pos = pos.value();
    CharacterLogicSystem::run_action(self, u"jump");
  }
  CharacterLogicSystem::self_fh = 0;
  CharacterLogicSystem::self_lr = 0;
  CharacterLogicSystem::self_hspeed = 0;
  CharacterLogicSystem::self_vspeed = 0;
  CharacterLogicSystem::run_unsit_chair(self);

  self.action_animate = true;
  self.tomb = std::nullopt;
  self.skill = std::nullopt;
}

void SceneSystemInstance::enter(uint32_t map_id) {
  enter_prepare();
  SceneSystemInstance::map_id = map_id;

  FootholdGameInstance::data = FootholdGameInstance::load(map_id);
  BackgrndGameInstance::load(map_id);
  LadderropeGameInstance::load(map_id);
  MobGameInstance::load(map_id);
  NpcGameInstance::load(map_id);
  ObjGameInstance::load(map_id);
  TrapGameInstance::load(map_id);
  PortalGameInstance::data = PortalGameInstance::load(map_id);
  ReactorGameInstance::load(map_id);
  TileGameInstance::load(map_id);
  TooltipGameInstance::load(map_id);
  SeatGameInstance::load(map_id);
  MinimapUiSystem::load();
  CharacterGameInstance::others.clear();
  EffectGameInstance::reset();
  CameraGameInstance::reset();
  StatusbarUiSystem::reset();
  BallGameInstance::reset();
  DropGameInstance::reset();
  auto bgm = MapInfoGameInstance::load_bgm(map_id);
  AudioGameInstance::load_backgrnd_audio(bgm);

  System::event_systems = {
      QuestAlarmUiSystem::event, MinimapUiSystem::event,
      QuickSlotUiSystem::event,  StatusbarUiSystem::event,
      BuffUiSystem::event,        CursorLogicSystem::event,
      KeyboardInputSystem::event,
  };

  System::logic_systems = {
      CameraLogicSystem::run,    BackgrndLogicSystem::run,
      BallLogicSystem::run,      EffectLogicSystem::run,
      CharacterLogicSystem::run, ChatballoonLogicSystem::run,
      DropLogicSystem::run,      MobLogicSystem::run,
      NpcLogicSystem::run,       PortalLogicSystem::run,
      AudioLogicSystem::run,     ObjLogicSystem::run,
      CursorLogicSystem::run,    SkillLogicSystem::run,
      ReactorLogicSystem::run,   ItemLogicSystem::run,
      TrapLogicSystem::run,
  };
  System::render_systems = {
      render_game,
      BuffUiSystem::render,
      StatusbarUiSystem::render,
      QuickSlotUiSystem::render,
      PopupTipUiSystem::render,
      MinimapUiSystem::render,
      QuestAlarmUiSystem::render,
      GainLogUiSystem::render,
      CursorRenderSystem::render,
  };
  FadeSystemInstance::enter_out();
  Window::delta_time = 0;
}

SDL_FPoint SceneSystemInstance::load_prepare_pos(const std::u16string &pn,
                                                   uint8_t index) {
  SDL_FPoint r;
  GamePortal *por;
  auto portals =
      PortalGameInstance::load(SceneSystemInstance::prepare_map_id);
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

void SceneSystemInstance::enter_prepare(uint32_t map_id,
                                          const std::u16string &pn, int i) {
  prepare_map_id = map_id;
  prepare_pos = load_prepare_pos(pn, i);

  fbs::ClientSceneT client_scene;
  client_scene.fade = false;
  client_scene.map_id = map_id;

  // 记录起始时间
  Window::dt_time_start = Window::dt_now;
  ClientRequest::send_to_host(client_scene);
}

void SceneSystemInstance::enter_fade() {
  auto g = CharacterGameInstance::self;
  g.pos = prepare_pos.value();

  CharacterLogicSystem::run_action(g, u"jump");
  auto c = ServerCharacterInstance::load_charactert(g);

  fbs::ClientSceneT client_scene;
  client_scene.fade = true;
  client_scene.map_id = prepare_map_id;
  client_scene.character = std::make_unique<fbs::CharacterT>(std::move(c));

  ClientRequest::send_to_host(client_scene);
}

void SceneSystemInstance::enter_quit() {
  LoginSystemInstance::enter();
  FadeSystemInstance::enter_out();
  FadeSystemInstance::enter_out();
}

void SceneSystemInstance::quit_prepare() {
  fbs::ClientSceneT client_scene;
  client_scene.fade = false;
  client_scene.map_id = 0;
  ClientRequest::send_to_host(client_scene);
  FadeSystemInstance::enter_in(SceneSystemInstance::enter_quit);
}

void SceneSystemInstance::enter_cash_shop() {
  CashShopSystemInstance::enter();
  FadeSystemInstance::enter_out();
  FadeSystemInstance::enter_out();
}

void SceneSystemInstance::cash_shop_prepare() {
  fbs::ClientSceneT client_scene;
  client_scene.fade = false;
  client_scene.map_id = 0;
  ClientRequest::send_to_host(client_scene);
  FadeSystemInstance::enter_in(SceneSystemInstance::enter_quit);
}