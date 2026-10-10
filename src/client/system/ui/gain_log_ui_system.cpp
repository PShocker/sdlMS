#include "gain_log_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/gain_log_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"

bool gain_log_ui_system::render() {
  auto &data = gain_log_game_instance::data;
  freetype::load_size(12);
  freetype::load_bold(false);
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_y = (screen_h - 168);
  for (int i = data.size() - 1; i >= 0; i--) {
    const auto &g_log = data[i];
    std::u16string text;
    switch (g_log.type) {
    case gain_enum::item: {
      auto itm_id = g_log.id;
      std::u16string item_name;
      auto num = g_log.num;
      auto num2 = std::to_string(num);
      auto num3 = std::u16string{num2.begin(), num2.end()};
      if (itm_id == u"00000000") {
        text = u"You have gained meso (+" + num3 + u")";
      } else if (item_game_instance::check_item(itm_id)) {
        item_name = item_game_instance::load_item_text(itm_id, u"name");
        auto item_type = item_game_instance::load_item_type(itm_id);
        text = item_name + u" x" + num3 + u" earned (" + item_type + u")";
      } else {
        item_name = equip_game_instance::load_equip_name(itm_id);
        text = item_name + u" x" + num3 + u" earned (Equip/Deco)";
      }
      break;
    }
    case gain_enum::experience: {
      auto exp_str = std::to_string(g_log.num);
      auto exp_str2 = std::u16string{exp_str.begin(), exp_str.end()};
      text = u"You received EXP (+" + exp_str2 + u")";
      break;
    }
    }
    if (g_log.destroy > window::dt_now) {
      float alpha = g_log.destroy - window::dt_now;
      alpha = (alpha / 8000) * 255;
      freetype::load_color(255, 255, 255, alpha);
      auto text_w = freetype::load_w(text);
      auto base_x = screen_w - text_w;
      freetype::draw_line(text, base_x, base_y);
      base_y -= freetype::load_lh();
    }
  }
  std::erase_if(data, [](game_gain_log &g_log) {
    auto destroy = g_log.destroy;
    return destroy <= window::dt_now;
  });
  return true;
}