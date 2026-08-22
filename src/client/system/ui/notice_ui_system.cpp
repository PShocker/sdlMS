#include "notice_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "package_ui_system.h"
#include "src/client/game/game_shop.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/shop_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/shop_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "text_input_ui_system.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

void notice_ui_system::render_backgrnd() {
  wz::Node *node;
  switch (type) {
  case notice_enum::worldmap_disable:
  case notice_enum::worldmap_teleport:
  case notice_enum::equip_no_ability:
  case notice_enum::equip_no_space:
  case notice_enum::shopbuy_no_meso:
  case notice_enum::shopbuy_no_space:
  case notice_enum::shopbuy_sell:
  case notice_enum::shopbuy_sell_no_num:
  case notice_enum::shopbuy: {
    node = wz_resource::ui->find(u"PopupWindow.img/Notice1");
    break;
  }
  case notice_enum::ap_inc:
  case notice_enum::shopbuy_sell_mul:
  case notice_enum::shopbuy_mul:
  case notice_enum::throw_mul: {
    node = wz_resource::ui->find(u"PopupWindow.img/Notice2");
    break;
  }
  }
  auto [w, h] = load_wh();
  auto t = wz_resource::load_texture(node->get_child(u"t"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  auto c = wz_resource::load_texture(node->get_child(u"c"));
  auto s = wz_resource::load_texture(node->get_child(u"s"));
  pos_rect = {
      pos.x,
      pos.y + t->h,
      static_cast<float>(c->w),
      static_cast<float>(h - t->h - s->h),
  };
  SDL_RenderTextureTiled(window::renderer, c, nullptr, 1, &pos_rect);

  pos_rect = {
      pos.x,
      pos.y + h - s->h,
      static_cast<float>(s->w),
      static_cast<float>(s->h),
  };
  SDL_RenderTexture(window::renderer, s, nullptr, &pos_rect);
}

void notice_ui_system::render_button() {
  std::vector<wz::Node *> buttons_node = {};
  std::vector<SDL_FRect> buttons_rect = {};
  auto [w, h] = load_wh();
  switch (type) {
  case notice_enum::worldmap_disable:
  case notice_enum::worldmap_teleport:
  case notice_enum::ap_inc:
  case notice_enum::shopbuy_sell:
  case notice_enum::shopbuy_sell_mul:
  case notice_enum::shopbuy_sell_no_num:
  case notice_enum::shopbuy:
  case notice_enum::shopbuy_mul:
  case notice_enum::shopbuy_no_meso:
  case notice_enum::shopbuy_no_space:
  case notice_enum::equip_no_ability:
  case notice_enum::throw_mul:
  case notice_enum::equip_no_space: {
    buttons_node = {
        wz_resource::ui->find(u"Basic.img/BtOK2"),
        wz_resource::ui->find(u"Basic.img/BtCancel2"),
    };
    buttons_rect = {
        {w - 105, h - 30, 47, 18},
        {w - 55, h - 30, 47, 18},
    };
    break;
  }
  }
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    auto &mouse_pos = window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = cursor_game_instance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = wz_resource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = wz_resource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = wz_resource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void notice_ui_system::render_text() {
  std::u16string text = u"";
  SDL_FPoint p;
  switch (type) {
  case notice_enum::shopbuy: {
    auto n = wz_resource::ms->get_root()->find(u"String.img/Notice/buyItem");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case notice_enum::shopbuy_mul: {
    auto n = wz_resource::ms->get_root()->find(u"String.img/Notice/buyItemMul");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case notice_enum::shopbuy_no_meso: {
    auto n = wz_resource::ms->get_root()->find(u"String.img/Notice/noMeso");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case notice_enum::shopbuy_no_space: {
    auto n = wz_resource::ms->get_root()->find(u"String.img/Notice/noSpace");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case notice_enum::equip_no_ability:
  case notice_enum::equip_no_space:
    break;
  case notice_enum::shopbuy_sell: {
    auto n = wz_resource::ms->get_root()->find(u"String.img/Notice/sellItem");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case notice_enum::shopbuy_sell_mul: {
    auto n =
        wz_resource::ms->get_root()->find(u"String.img/Notice/sellItemMul");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case notice_enum::shopbuy_sell_no_num: {
    auto n = wz_resource::ms->get_root()->find(u"String.img/Notice/sellNoNum");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case notice_enum::ap_inc: {
    break;
  }
  case notice_enum::worldmap_disable: {
    auto n =
        wz_resource::ms->get_root()->find(u"String.img/Notice/worldMapDis");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case notice_enum::worldmap_teleport: {
    auto n =
        wz_resource::ms->get_root()->find(u"String.img/Notice/worldMapTel");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case notice_enum::throw_mul: {
    auto n = wz_resource::ms->get_root()->find(u"String.img/Notice/throwMul");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  default: {
    break;
  }
  }
  if (!text.empty()) {
    freetype::load_aligned(true);
    freetype::load_size(12);
    freetype::load_color(255, 255, 255, 255);
    freetype::draw_str(text, pos.x + p.x, pos.y + p.y, 240, 1.3);
  }
}

void notice_ui_system::render_input() {
  switch (type) {
  case notice_enum::ap_inc:
  case notice_enum::shopbuy_sell_mul:
  case notice_enum::shopbuy_mul:
  case notice_enum::throw_mul: {
    text_input_ui_system::render(text, 5, 5);
    break;
  }
  default: {
    break;
  }
  }
}

bool notice_ui_system::render() {
  render_backgrnd();
  render_button();
  render_text();
  render_input();
  return true;
}

void notice_ui_system::open() {
  keyboard_input_system::reset();

  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    switch (type) {
    case notice_enum::ap_inc:
    case notice_enum::shopbuy_sell:
    case notice_enum::shopbuy_sell_mul:
    case notice_enum::shopbuy_mul:
    case notice_enum::throw_mul: {
      int num;
      if (std::any_cast<std::nullptr_t>(&notice_ui_system::data)) {
        num = 1;
      } else {
        auto p = std::any_cast<std::polymorphic<game_item> *>(
            notice_ui_system::data);
        auto num = item_game_instance::load_item_num(*p);
      }
      auto tmp = std::to_string(num);
      text = {
          .max_size = 12,
          .text = {tmp.begin(), tmp.end()},
          .composition = {},
          .disable = false,
          .active = false,
          .r =
              SDL_Rect{
                  static_cast<int>(pos.x + 18),
                  static_cast<int>(pos.y + 40),
                  230,
                  30,
              },
          .font_color = {0, 0, 0, 255},
          .cur_color = {0, 0, 0, 255},
          .font_size = 12,
      };
      text.type.reset();
      text.type.set(text_input::digit);
      text.cur = 1;
      text_input_ui_system::active(text);
      break;
    }
    default: {
      break;
    }
    }

    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void notice_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

SDL_FPoint notice_ui_system::load_wh() {
  switch (type) {
  case notice_enum::worldmap_disable:
  case notice_enum::worldmap_teleport:
  case notice_enum::shopbuy_sell:
  case notice_enum::shopbuy_sell_no_num:
  case notice_enum::shopbuy:
  case notice_enum::shopbuy_no_meso:
  case notice_enum::shopbuy_no_space: {
    return {266, 116};
  }
  case notice_enum::ap_inc:
  case notice_enum::shopbuy_sell_mul:
  case notice_enum::shopbuy_mul:
  case notice_enum::throw_mul:
    return {266, 119};
  case notice_enum::equip_no_ability:
  case notice_enum::equip_no_space:
    break;
  }
  return {0, 0};
}

void notice_ui_system::event_close() { close(); }

void notice_ui_system::event_button_shopbuy() {
  auto p = std::any_cast<const game_shop_item *>(notice_ui_system::data);
  auto itm = p->item;
  switch (type) {
  case notice_enum::shopbuy: {
    shop_ui_system::add_item(itm);
    package_game_instance::meso -= p->price;
    break;
  }
  case notice_enum::shopbuy_mul: {
    auto num = std::stoi(std::string{text.text.begin(), text.text.end()});
    switch (itm->type) {
    case item_enum::consume: {
      auto &consume = static_cast<game_consume_item &>(*itm);
      consume.num = num;
      break;
    }
    case item_enum::etc: {
      auto &etc = static_cast<game_etc_item &>(*itm);
      etc.num = num;
      break;
    }
    default: {
      break;
    }
    }
    auto price = p->price * num;
    if (package_game_instance::meso < price) {
      type = notice_enum::shopbuy_no_meso;
      return;
      break;
    }
    auto buy_num = std::stoi(std::string{text.text.begin(), text.text.end()});
    if (shop_ui_system::active_tab[0] == 1) {
      auto &must = shop_ui_system::must;
      auto &must_itm = must[shop_ui_system::active_item[0].value()];
      auto itm_num = item_game_instance::load_item_num(must_itm.item);
      if (buy_num > itm_num) {
        type = notice_enum::shopbuy_sell_no_num;
        return;
      }
    }
    if (shop_ui_system::add_item(itm)) {
      package_game_instance::meso -= price;
      if (shop_ui_system::active_tab[0] == 1) {
        auto &must = shop_ui_system::must;
        auto &must_itm = must[shop_ui_system::active_item[0].value()];
        shop_ui_system::dec_item_num(must_itm.item, buy_num);
      }
    } else {
      type = notice_enum::shopbuy_no_space;
      return;
    }
    break;
  }
  default: {
    break;
  }
  }
  if (shop_ui_system::active_tab[0] == 1) {
    auto &must = shop_ui_system::must;
    auto &itm = must[shop_ui_system::active_item[0].value()].item;
    if (item_game_instance::load_item_num(itm) == 0) {
      must.erase(must.begin() + shop_ui_system::active_item[0].value());
    }
    shop_ui_system::pages[0] = 0;
  }
  shop_ui_system::active_tab[1] = (int)itm->type;
  shop_ui_system::pages[1] = 0;
  shop_ui_system::active_item = {};
  close();
}

void notice_ui_system::event_button_shopbuy_sell() {
  auto p = std::any_cast<std::polymorphic<game_item> *>(notice_ui_system::data);
  game_shop_item gst = shop_game_instance::load_shop_item((*p)->id);
  gst.item = *p;
  switch (type) {
  case notice_enum::shopbuy_sell: {
    auto itm_info = item_game_instance::load_item_info(gst.item->id, 0);
    auto unitPrice = itm_info->get_child(u"unitPrice");
    auto num = item_game_instance::load_item_num(gst.item);
    if (unitPrice) {
      auto u_price = static_cast<wz::Property<double> *>(unitPrice)->get();
      package_game_instance::meso += u_price * num;
    } else {
      package_game_instance::meso += gst.price;
    }
    shop_ui_system::dec_item_num(*p, num);
    break;
  }
  case notice_enum::shopbuy_sell_mul: {
    auto num = item_game_instance::load_item_num(gst.item);
    auto sell_num = std::stoi(std::string{text.text.begin(), text.text.end()});
    if (sell_num > num) {
      type = notice_enum::shopbuy_sell_no_num;
      return;
    }
    package_game_instance::meso += gst.price * num;
    shop_ui_system::dec_item_num(*p, sell_num);
    shop_ui_system::add_item_num(gst.item, (sell_num - num));
    break;
  }
  default: {
    break;
  }
  }
  shop_ui_system::add_must_item(gst.item);
  shop_ui_system::active_tab[0] = 1;
  shop_ui_system::active_item = {};
  shop_ui_system::pages = {};
  close();
}

void notice_ui_system::event_button_ap_inc() {
  auto num = std::stoi(std::string{text.text.begin(), text.text.end()});
  if (num > character_stat_game_instance::remain_ap) {
    type = notice_enum::shopbuy_sell_no_num;
    return;
  }
  auto p = std::any_cast<uint32_t *>(notice_ui_system::data);
  *p = *p + num;
  character_stat_game_instance::remain_ap -= num;
  close();
}

void notice_ui_system::event_button_worldmap_teleport() {
  auto p = std::any_cast<uint32_t>(notice_ui_system::data);
  scene_system_instance::enter_prepare(p, u"sp", 0);
}

void notice_ui_system::event_button_throw_mul() {
  int itm_num;
  std::u16string itm_id;
  if (std::any_cast<std::nullptr_t>(&notice_ui_system::data)) {
    itm_id = u"0";
    itm_num = package_game_instance::meso;
  } else {
    auto p =
        std::any_cast<std::polymorphic<game_item> *>(notice_ui_system::data);
    itm_id = (*p)->id;
    itm_num = item_game_instance::load_item_num(*p);
  }
  auto throw_num = std::stoi(std::string{text.text.begin(), text.text.end()});
  if (throw_num > itm_num) {
    type = notice_enum::shopbuy_sell_no_num;
    cursor_game_instance::cursor_hand = std::nullopt;
    return;
  } else {
    DropT dt;
    ItemT it;
    it.item_id = std::stoi(std::string{itm_id.begin(), itm_id.end()});
    it.item_num = throw_num;
    dt.drop.Set(it);

    dt.x1 = character_game_instance::self.pos.x;
    dt.y1 = character_game_instance::self.pos.y;

    dt.page = character_game_instance::self.page;

    ClientCharacterDropT cct;
    cct.map_id = scene_system_instance::map_id;
    cct.payload = std::make_unique<DropT>(dt);
    client_request::send_to_host(cct);

    cursor_game_instance::cursor_hand_net = {
        .type = cursor_game_instance::drop,
    };
    close();
  }
}

void notice_ui_system::event_button_throw_close() {
  cursor_game_instance::cursor_hand = std::nullopt;
  close();
}

bool notice_ui_system::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> buttons_rect;
  std::vector<std::function<void()>> func = {};
  auto [w, h] = load_wh();
  buttons_rect = {
      {w - 105, h - 30, 47, 18},
      {w - 55, h - 30, 47, 18},
  };
  switch (type) {
  case notice_enum::shopbuy:
  case notice_enum::shopbuy_mul: {
    func = {event_button_shopbuy, close};
    break;
  }
  case notice_enum::ap_inc: {
    func = {event_button_shopbuy_sell, close};
    break;
  }
  case notice_enum::worldmap_teleport: {
    func = {event_button_worldmap_teleport, close};
    break;
  }
  case notice_enum::shopbuy_sell:
  case notice_enum::shopbuy_sell_mul: {
    func = {event_button_shopbuy_sell, close};
    break;
  }
  case notice_enum::throw_mul: {
    func = {event_button_throw_mul, event_button_throw_close};
    break;
  }
  default: {
    func = {close, close};
    break;
  }
  }
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      func[i]();
      audio_game_instance::load_audio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }
  return false;
}

void notice_ui_system::event_input(SDL_Event *event) {
  switch (type) {
  case notice_enum::ap_inc:
  case notice_enum::shopbuy_sell_mul:
  case notice_enum::shopbuy_mul:
  case notice_enum::throw_mul: {
    text_input_ui_system::event(event, text);
  }
  default: {
    break;
  }
  }
}

bool notice_ui_system::event(SDL_Event *event) {
  event_input(event);
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      event_close();
      if (type == notice_enum::throw_mul) {
        cursor_game_instance::cursor_hand = std::nullopt;
      }
      break;
    }
    case SDL_SCANCODE_RETURN: {
      switch (type) {
      case notice_enum::shopbuy:
      case notice_enum::shopbuy_mul: {
        event_button_shopbuy();
        break;
      }
      case notice_enum::shopbuy_sell:
      case notice_enum::shopbuy_sell_mul: {
        event_button_shopbuy_sell();
        break;
      }
      case notice_enum::ap_inc: {
        break;
      }
      case notice_enum::worldmap_teleport: {
        event_button_worldmap_teleport();
        break;
      }
      case notice_enum::throw_mul: {
        event_button_throw_mul();
        break;
      }
      default: {
        close();
        break;
      }
      }
      break;
    }
    default: {
      break;
    }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        event_button(event);
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    break;
  }
  default: {
    break;
  }
  }

  return false;
}

bool notice_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}