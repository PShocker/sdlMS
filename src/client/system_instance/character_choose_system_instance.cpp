#include "character_choose_system_instance.h"
#include "game_save_system_instance.h"
#include "login_system_instance.h"
#include "src/client/game/game_animate.h"
#include "src/client/game_instance/backgrnd_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/obj_game_instance.h"
#include "src/client/game_instance/tile_game_instance.h"
#include "src/client/system/logic/backgrnd_logic_system.h"
#include "src/client/system/logic/cursor_logic_system.h"
#include "src/client/system/logic/obj_logic_system.h"
#include "src/client/system/logic/sound_logic_system.h"
#include "src/client/system/render/backgrnd_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/obj_render_system.h"
#include "src/client/system/render/tile_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_choose_ui_system.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <ranges>

void character_choose_system_instance::enter(const std::string &login) {
  game_save_system_instance::load_save(login);
  // 进一步解析game_save
  auto character_size = game_save_system_instance::save.characters.size();
  character_choose_ui_system::characters.resize(character_size);
  for (int i = 0; i < character_size; i++) {
    auto sc = game_save_system_instance::save.characters[i];
    character_choose_ui_system::characters[i] = sc.character;
  }
  enter();
}

void character_choose_system_instance::enter() {
  auto fn = &login_system_instance::render_game;
  if (!std::ranges::contains(system::render_systems, fn)) {
    static auto image = wz_resource::ui->find(u"MapLogin.img");
    backgrnd_game_instance::load(image);
    obj_game_instance::load(image);
    tile_game_instance::load(image);
  }
  system::logic_systems = {
      backgrnd_logic_system::run,      obj_logic_system::run,
      sound_logic_system::run,         cursor_logic_system::run,
      character_choose_ui_system::run,
  };
  system::render_systems = {
      login_system_instance::render_game,
      character_choose_ui_system::render,
      cursor_render_system::render,
  };
  system::event_systems = {
      character_choose_ui_system::event,
  };
  auto &camera = camera_game_instance::camera;
  camera.x = -80 - camera.w / 2;
  camera.y = -479 - camera.h / 2;

  game_animate ani{.ani_delay = {250, 50, 50, 0}};
  auto character_size = character_choose_ui_system::characters.size();
  character_choose_ui_system::boards.assign(character_size, ani);
}