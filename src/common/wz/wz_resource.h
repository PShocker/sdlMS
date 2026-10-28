#ifndef SDLMS_SRC_COMMON_WZ_WZ_RESOURCE_H_
#define SDLMS_SRC_COMMON_WZ_WZ_RESOURCE_H_

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Files.h>
#include <wz/Node.h>

#include <any>
#include <cstdint>
#include <flat_map>
#include <string>

namespace sdlms {

class WzResource {
 public:
  static inline wz::Files* character_;
  static inline wz::Files* effect_;
  static inline wz::Files* etc_;
  static inline wz::Files* item_;
  static inline wz::Files* map_;
  static inline wz::Files* mob_;
  static inline wz::Files* morph_;
  static inline wz::Files* npc_;
  static inline wz::Files* quest_;
  static inline wz::Files* reactor_;
  static inline wz::Files* skill_;
  static inline wz::Files* sound_;
  static inline wz::Files* string_;
  static inline wz::Files* taming_mob_;
  static inline wz::Files* ui_;

  // patch
  static inline wz::File* ms_;

  static void Init();
  static std::string LoadMapPath(uint32_t map_id);
  static wz::Node* LoadMapNode(uint32_t map_id);
  static SDL_Texture* LoadTexture(wz::Node* node);
  static SDL_FPoint LoadFpoint(wz::Node* node);

 private:
  static void InitKey();
  static void InitFiles();
  static wz::Node* LoadRealNodeTexture(wz::Node* node);
  static SDL_Texture* LoadNodeTexture(wz::Node* node);

  static inline std::flat_map<wz::Node*, SDL_Texture*> texture_cache_;
  static inline std::flat_map<uint32_t, wz::Node*> map_node_cache_;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_COMMON_WZ_WZ_RESOURCE_H_
