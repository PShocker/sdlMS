#include "src/client/game_instance/mob_game_instance.h"

#include "src/client/game/game_effect.h"
#include "src/client/game/game_mob.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <cstdint>
#include <flat_map>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace sdlms {

std::u16string MobGameInstance::LoadMobName(const std::u16string& id) {
  std::u16string r = u"";
  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });

  std::u16string result(view.begin(), view.end());
  auto str_node = WzResource::string_->find(u"Mob.img/" + result + u"/name");
  auto str = static_cast<wz::Property<std::u16string>*>(str_node)->get();
  return str;
}

wz::Node* MobGameInstance::LoadMobInfo(const std::u16string& id) {
  static std::flat_map<std::u16string, wz::Node*> cache;
  if (!cache.contains(id)) {
    auto mob_node = LoadLinkMobNode(id);
    cache[id] = mob_node->get_child(u"info");
  }
  return cache.at(id);
}

wz::Node* MobGameInstance::LoadLinkMobNode(const std::u16string& id) {
  auto mob_node = WzResource::mob_->find(id + u".img");
  while (mob_node->find(u"info/link")) {
    auto link_node = mob_node->find(u"info/link");
    auto link = static_cast<wz::Property<std::u16string>*>(link_node)->get();
    mob_node = WzResource::mob_->find(link + u".img");
  }
  return mob_node;
}

void MobGameInstance::Load(uint32_t map_id) {
  data_ = {};
}

}  // namespace sdlms
