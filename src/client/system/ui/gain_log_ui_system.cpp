#include "gain_log_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/gain_log_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include <format>
#include <string>

void GainLogUiSystem::RenderText() {
  auto &data = GainLogGameInstance::data;
  Freetype::LoadSize(12);
  auto screen_w = CameraGameInstance::camera.w;
  auto screen_h = CameraGameInstance::camera.h;
  auto base_y = (screen_h - 168);
  for (int i = data.size() - 1; i >= 0; i--) {
    const auto &g_log = data[i];
    std::u16string text;
    switch (g_log.type) {
    case GainEnum::kItem: {
      auto itm_id = g_log.id;
      std::u16string item_name;
      auto num = g_log.num;
      auto num2 = std::to_string(num);
      auto num3 = std::u16string{num2.begin(), num2.end()};
      if (itm_id == u"00000000") {
        text = u"You have gained meso (+" + num3 + u")";
      } else if (ItemGameInstance::CheckItem(itm_id)) {
        item_name = ItemGameInstance::LoadItemText(itm_id, u"name");
        auto item_type = ItemGameInstance::LoadItemType(itm_id);
        text = item_name + u" x" + num3 + u" earned (" + item_type + u")";
      } else {
        item_name = EquipGameInstance::LoadEquipName(itm_id);
        text = item_name + u" x" + num3 + u" earned (Equip/Deco)";
      }
      break;
    }
    case GainEnum::kExperience: {
      auto exp_str = std::to_string(g_log.num);
      auto exp_str2 = std::u16string{exp_str.begin(), exp_str.end()};
      text = u"You received EXP (+" + exp_str2 + u")";
      break;
    }
    }
    if (g_log.destroy > Window::dt_now) {
      float alpha = g_log.destroy - Window::dt_now;
      alpha = (alpha / 10000) * 255;
      Freetype::LoadColor(255, 255, 255, alpha);
      auto text_w = Freetype::LoadW(text);
      auto base_x = screen_w - text_w;
      Freetype::DrawLine(text, base_x, base_y);
      base_y -= Freetype::LoadLh();
    }
  }
}

void GainLogUiSystem::Destory() {
  auto &data = GainLogGameInstance::data;
  std::erase_if(data, [](GameGainLog &g_log) {
    auto destroy = g_log.destroy;
    return destroy <= Window::dt_now;
  });
}

bool GainLogUiSystem::Render() {
  RenderText();
  Destory();

  return true;
}