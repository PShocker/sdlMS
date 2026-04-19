#include "backgrnd_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_backgrnd.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <cstdlib>
#include <string>

void backgrnd_game_instance::load(uint32_t map_id) {
  front.clear();
  back.clear();
  auto map_node = wz_resource::load_map_node(map_id);
  auto map_back_node = map_node->get_child(u"back");
  for (auto [key, val] : *map_back_node->get_children()) {

    auto backgrnd_node = val[0];
    game_backgrnd g_backgrnd;
    auto id = std::stoi(std::string{key.begin(), key.end()});

    auto bs = static_cast<wz::Property<std::u16string> *>(
                  backgrnd_node->get_child(u"bS"))
                  ->get();

    auto x =
        static_cast<wz::Property<int> *>(backgrnd_node->get_child(u"x"))->get();
    auto y =
        static_cast<wz::Property<int> *>(backgrnd_node->get_child(u"y"))->get();

    g_backgrnd.pos = SDL_FPoint{static_cast<float>(x), static_cast<float>(y)};

    g_backgrnd.cx =
        static_cast<wz::Property<int> *>(backgrnd_node->get_child(u"cx"))
            ->get();
    g_backgrnd.cy =
        static_cast<wz::Property<int> *>(backgrnd_node->get_child(u"cy"))
            ->get();

    g_backgrnd.rx =
        static_cast<wz::Property<int> *>(backgrnd_node->get_child(u"rx"))
            ->get();
    g_backgrnd.ry =
        static_cast<wz::Property<int> *>(backgrnd_node->get_child(u"ry"))
            ->get();

    auto type =
        static_cast<wz::Property<int> *>(backgrnd_node->get_child(u"type"))
            ->get();

    enum { NORMAL, HTILED, VTILED, TILED, HMOVEA, VMOVEA, HMOVEB, VMOVEB };

    switch (type) {
    case NORMAL:
      g_backgrnd.htile = false;
      g_backgrnd.vtile = false;
      break;
    case HTILED:
    case HMOVEA:
      g_backgrnd.htile = true;
      g_backgrnd.vtile = false;
      break;
    case VTILED:
    case VMOVEA:
      g_backgrnd.htile = false;
      g_backgrnd.vtile = true;
      break;
    case TILED:
    case HMOVEB:
    case VMOVEB:
      g_backgrnd.htile = true;
      g_backgrnd.vtile = true;
      break;
    }
    switch (type) {
    case HMOVEA:
    case HMOVEB:
      g_backgrnd.hspeed = true;
      g_backgrnd.vspeed = false;
      break;
    case VMOVEA:
    case VMOVEB:
      g_backgrnd.hspeed = false;
      g_backgrnd.vspeed = true;
      break;
    default:
      g_backgrnd.hspeed = false;
      g_backgrnd.vspeed = false;
      break;
    }

    g_backgrnd.ani =
        static_cast<wz::Property<int> *>(backgrnd_node->get_child(u"ani"))
            ->get();

    std::u16string no2;
    if (backgrnd_node->get_child(u"no")->type == wz::Type::Int) {
      auto no =
          static_cast<wz::Property<int> *>(backgrnd_node->get_child(u"no"))
              ->get();
      auto no1 = std::to_string(no);
      no2 = std::u16string{no1.begin(), no1.end()};
    } else {
      no2 = static_cast<wz::Property<std::u16string> *>(
                backgrnd_node->get_child(u"no"))
                ->get();
    }
    switch (g_backgrnd.ani) {
    case 0: {
      g_backgrnd.path = u"Back/" + bs + u".img/back/" + no2;
      break;
    }
    case 1: {
      g_backgrnd.path = u"Back/" + bs + u".img/ani/" + no2;
      break;
    }
    default: {
      std::abort();
    }
    }

    auto f =
        static_cast<wz::Property<int> *>(backgrnd_node->get_child(u"f"))->get();
    if (f == 1) {
      front.insert({id, g_backgrnd});
    } else {
      back.insert({id, g_backgrnd});
    }
  }
}