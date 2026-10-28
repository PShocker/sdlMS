#include "clock_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
namespace sdlms {


void ClockGameInstance::Load(wz::Node *image) {
  data_ = {};
  if (auto n = image->get_child(u"clock")) {
    SDL_FRect r;
    r.x = static_cast<wz::Property<int> *>(n->get_child(u"x"))->get();
    r.y = static_cast<wz::Property<int> *>(n->get_child(u"y"))->get();
    r.w = static_cast<wz::Property<int> *>(n->get_child(u"width"))->get();
    r.h = static_cast<wz::Property<int> *>(n->get_child(u"height"))->get();
    data_.push_back(r);
  }
}

void ClockGameInstance::Load(uint32_t map_id) {
  auto map_node = WzResource::LoadMapNode(map_id);
  Load(map_node);
}

}  // namespace sdlms
