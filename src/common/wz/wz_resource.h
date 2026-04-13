#pragma once
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
  static inline wz::Files *uI;

  static inline void init();
  static wz::Node *load_map_node();

private:
  static void init_key();
  static void init_files();
};