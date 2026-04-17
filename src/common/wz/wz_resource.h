#pragma once
#include "SDL3/SDL_render.h"
#include <any>
#include <cstdint>
#include <flat_map>
#include <string>
#include <wz/Files.h>
#include <wz/Node.h>

class wz_resource {
public:
  static inline wz::Files *character;
  static inline wz::Files *effect;
  static inline wz::Files *etc;
  static inline wz::Files *item;
  static inline wz::Files *map;
  static inline wz::Files *mob;
  static inline wz::Files *morph;
  static inline wz::Files *npc;
  static inline wz::Files *quest;
  static inline wz::Files *reactor;
  static inline wz::Files *skill;
  static inline wz::Files *sound;
  static inline wz::Files *string;
  static inline wz::Files *taming_mob;
  static inline wz::Files *ui;

  static void init();
  static std::string load_map_path(uint32_t map_id);
  static wz::Node *load_map_node(uint32_t map_id);
  static SDL_Texture *load_texture(wz::Node *node);

private:
  static void init_key();
  static void init_files();
  static wz::Node *load_real_node_texture(wz::Node *node);
  static SDL_Texture *load_node_texture(wz::Node *node);

  static inline std::flat_map<wz::Node *, SDL_Texture *> texture_cache;
  static inline std::flat_map<uint32_t, wz::Node *> map_node_cache;
};