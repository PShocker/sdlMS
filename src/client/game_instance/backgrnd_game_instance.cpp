#include "src/client/game_instance/backgrnd_game_instance.h"

#include "src/client/game/game_backgrnd.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <wz/Property.h>
#include <wz/Wz.h>

#include <cstdlib>
#include <string>

namespace sdlms {

void BackgrndGameInstance::Load(wz::Node* image) {
  front_ = {};
  back_ = {};

  auto map_node = image;
  auto map_back_node = map_node->get_child(u"back");
  for (auto [key, val] : *map_back_node->get_children()) {
    auto backgrnd_node = val[0];
    GameBackgrnd g_backgrnd;
    auto id = std::stoi(std::string{key.begin(), key.end()});

    auto bs = static_cast<wz::Property<std::u16string>*>(
                  backgrnd_node->get_child(u"bS"))
                  ->get();
    if (bs == u"") {
      continue;
    }

    auto x =
        static_cast<wz::Property<int>*>(backgrnd_node->get_child(u"x"))->get();
    auto y =
        static_cast<wz::Property<int>*>(backgrnd_node->get_child(u"y"))->get();

    g_backgrnd.pos = SDL_FPoint{static_cast<float>(x), static_cast<float>(y)};

    g_backgrnd.cx =
        static_cast<wz::Property<int>*>(backgrnd_node->get_child(u"cx"))->get();
    g_backgrnd.cy =
        static_cast<wz::Property<int>*>(backgrnd_node->get_child(u"cy"))->get();

    g_backgrnd.rx =
        static_cast<wz::Property<int>*>(backgrnd_node->get_child(u"rx"))->get();
    g_backgrnd.ry =
        static_cast<wz::Property<int>*>(backgrnd_node->get_child(u"ry"))->get();

    auto type =
        static_cast<wz::Property<int>*>(backgrnd_node->get_child(u"type"))
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
        static_cast<wz::Property<int>*>(backgrnd_node->get_child(u"ani"))
            ->get();

    std::u16string no2;
    if (backgrnd_node->get_child(u"no")->type == wz::Type::Int) {
      auto no = static_cast<wz::Property<int>*>(backgrnd_node->get_child(u"no"))
                    ->get();
      auto no1 = std::to_string(no);
      no2 = std::u16string{no1.begin(), no1.end()};
    } else {
      no2 = static_cast<wz::Property<std::u16string>*>(
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

    SDL_Texture* texture;
    auto back_node = WzResource::map_->find(g_backgrnd.path);
    if (g_backgrnd.ani) {
      auto index =
          std::to_string(g_backgrnd.ani_index % back_node->children_count());
      back_node = back_node->get_child(index);
    }
    texture = WzResource::LoadTexture(back_node);

    g_backgrnd.cx = g_backgrnd.cx == 0 ? texture->w : g_backgrnd.cx;
    g_backgrnd.cy = g_backgrnd.cy == 0 ? texture->h : g_backgrnd.cy;

    g_backgrnd.flip =
        static_cast<wz::Property<int>*>(backgrnd_node->get_child(u"f"))->get();

    auto fnt =
        static_cast<wz::Property<int>*>(backgrnd_node->get_child(u"front"))
            ->get();
    if (fnt == 1) {
      front_.insert({id, g_backgrnd});
    } else {
      back_.insert({id, g_backgrnd});
    }
  }
}

void BackgrndGameInstance::Load(uint32_t map_id) {
  auto map_node = WzResource::LoadMapNode(map_id);
  Load(map_node);
}

}  // namespace sdlms
