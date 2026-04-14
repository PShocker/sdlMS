#include "sound_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

void sound_game_instance::load_map_sound(uint32_t map_id) {
  auto map_node = wz_resource::load_map_node(map_id);
  auto bgm_node = map_node->find(u"info/bgm");
  backgrnd_sound.path =
      static_cast<wz::Property<std::u16string> *>(bgm_node)->get();
  backgrnd_sound.delay = 0;
  backgrnd_sound.offset = 0;
}