#include "chatacter_create_system_instance.h"
#include "login_system_instance.h"
#include "src/client/game_instance/backgrnd_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/obj_game_instance.h"
#include "src/client/game_instance/tile_game_instance.h"
#include "src/client/system/logic/backgrnd_logic_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/cursor_logic_system.h"
#include "src/client/system/logic/obj_logic_system.h"
#include "src/client/system/logic/sound_logic_system.h"
#include "src/client/system/render/backgrnd_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/obj_render_system.h"
#include "src/client/system/render/tile_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_create_ui_system.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <ranges>

void chatacter_create_system_instance::enter() {
  auto fn = &login_system_instance::render_game;
  if (!std::ranges::contains(system::render_systems, fn)) {
    static auto image = wz_resource::ui->find(u"MapLogin.img");
    backgrnd_game_instance::load(image);
    obj_game_instance::load(image);
    tile_game_instance::load(image);
  }
  auto &g_character = character_create_ui_system::g_character;
  character_game_instance::add_body(g_character, u"00002000");
  character_game_instance::add_head(g_character, u"00012000");
  character_game_instance::add_coat(g_character, u"01040002");
  character_game_instance::add_pants(g_character, u"01060001");
  character_game_instance::add_cap(g_character, u"01002005");
  character_game_instance::add_weapon(g_character, u"01372006");
  character_game_instance::add_face(g_character, u"00020000");
  character_game_instance::add_hair(g_character, u"00030000");
  character_logic_system::run_stand_action(g_character);
  g_character.flip = 1;
  
  system::logic_systems = {
      backgrnd_logic_system::run,
      obj_logic_system::run,
      sound_logic_system::run,
      cursor_logic_system::run,
  };
  system::render_systems = {
      login_system_instance::render_game,
      character_create_ui_system::render,
      cursor_render_system::render,
  };
  system::event_systems = {
      character_create_ui_system::event,
  };
  auto &camera = camera_game_instance::camera;
  camera.x = -80 - camera.w / 2;
  camera.y = -1294 - camera.h / 2;
}