#include "wz_resource.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/window/window.h"
#include "wz/Files.h"
#include "wz/Key.h"
#include "wz/Property.h"
#include "wz/Types.h"
#include "wz/Wz.h"
#include <array>
#include <cstdint>
#include <cstdlib>
#include <flat_map>
#include <string>

void wz_resource::init_key() {
  std::array<uint8_t, 4> iv{0x4d, 0x23, 0xc7, 0x2b};
  std::vector<uint8_t> aes_key = {
      0x13, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
      0x00, 0xB4, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x0F, 0x00,
      0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x52, 0x00, 0x00, 0x00};
  wz::init_key(iv, aes_key);
}

void wz_resource::init_files() {
  character = new wz::Files("Character/Character");
}

void wz_resource::init() {
  init_key();
  init_files();
}

std::string wz_resource::load_map_path(uint32_t map_id) {
  auto s = std::to_string(map_id);
  if (s.size() < 9) {
    s.insert(0, 9 - s.size(), '0');
  }
  return s;
}

wz::Node *wz_resource::load_map_node(uint32_t map_id) {
  auto s = load_map_path(map_id);
  std::string path =
      "Map" + std::to_string(map_id / 100000000) + "/" + s + ".img";
  return map->find(path);
}

SDL_Texture *wz_resource::load_node_texture(wz::Node *node) {
  SDL_Texture *texture = nullptr;
  if (node->type != wz::Type::Canvas) {
    std::abort();
  }
  auto canvas = static_cast<wz::Property<wz::WzCanvas> *>(node);
  auto raw_data = canvas->get_raw_data();
  auto h = canvas->get().height;
  auto w = canvas->get().width;
  auto format = canvas->get().format + canvas->get().format2;
  std::vector<uint8_t> pixel;
  switch (format) {
  case 1: {
    pixel = raw_data;
    texture = SDL_CreateTexture(window::renderer, SDL_PIXELFORMAT_ARGB4444,
                                SDL_TEXTUREACCESS_STATIC, w, h);
    SDL_UpdateTexture(texture, NULL, pixel.data(), w * sizeof(Uint16));
    break;
  }
  case 2: {
    pixel = raw_data;
    texture = SDL_CreateTexture(window::renderer, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STATIC, w, h);
    SDL_UpdateTexture(texture, NULL, pixel.data(), w * sizeof(Uint32));
    break;
  }
  case 517: // rgb565压缩缩略图
  {
    pixel.resize(w * h * 2, 0);
    int lineIndex = 0;
    for (int j0 = 0, j1 = h / 16; j0 < j1; j0++) {
      int dstIndex = lineIndex;
      for (int i0 = 0, i1 = h / 16; i0 < i1; i0++) {
        int idx = (i0 + j0 * i1) * 2;
        unsigned char b0 = raw_data[idx];
        unsigned char b1 = raw_data[idx + 1];
        for (int k = 0; k < 16; k++) {
          pixel[dstIndex++] = b0;
          pixel[dstIndex++] = b1;
        }
      }
      for (int k = 1; k < 16; k++) {
        for (int m = 0; m < w * 2; m++) {
          pixel[dstIndex + m] = pixel[lineIndex + m];
        }
        dstIndex += w * 2;
      }
      lineIndex += w * 32;
    }
    texture = SDL_CreateTexture(window::renderer, SDL_PIXELFORMAT_RGB565,
                                SDL_TEXTUREACCESS_STATIC, w, h);
    SDL_UpdateTexture(texture, NULL, pixel.data(), w * sizeof(Uint16));
    break;
  }
  default: {
    std::abort();
  }
  }
  return texture;
}

wz::Node *wz_resource::load_real_node_texture(wz::Node *node) {
  auto outlink = node->get_child("_outlink");
  if (outlink != nullptr) {
    auto outlink2 = static_cast<wz::Property<std::u16string> *>(outlink)->get();
    const static std::flat_map<std::u16string, wz::Files *> root_map = {
        {u"Character", character},
        {u"Effect", effect},
        {u"Etc", etc},
        {u"Item", item},
        {u"Map", map},
        {u"Mob", mob},
        {u"Morph", morph},
        {u"Npc", npc},
        {u"Quest", quest},
        {u"Reactor", reactor},
        {u"Skill", skill},
        {u"Sound", sound},
        {u"String", string},
        {u"TamingMob", taming_mob},
        {u"UI", ui},
    };
    size_t pos = outlink2.find_first_of(u'/');
    if (pos == std::string::npos) {
      std::abort();
    }
    auto first = outlink2.substr(0, pos);
    auto second = outlink2.substr(pos + 1);
    node = root_map.at(first)->find(second);
  }
  return node;
}

SDL_Texture *wz_resource::load_texture(wz::Node *node) {
  if (!texture_cache.contains(node)) {
    auto real_node = load_real_node_texture(node);
    auto texture = load_node_texture(real_node);
    texture_cache[node] = texture;
  }
  return texture_cache.at(node);
}