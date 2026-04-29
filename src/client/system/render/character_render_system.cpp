#include "character_render_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/window/window.h"
#include <flat_map>
#include <string>
#include <vector>

bool character_render_system::render(game_character &g_character) {
  std::flat_multimap<std::u16string, const character_avatar *> renders;
  std::vector<const character_avatar_render *> render_parts;
  auto &camera = camera_game_instance::camera;
  const auto &head = character_game_instance::avatar_data.at(g_character.head);
  const auto &body = character_game_instance::avatar_data.at(g_character.body);
  const auto &hair = character_game_instance::avatar_data.at(g_character.hair);
  render_parts.push_back(&head);
  render_parts.push_back(&body);
  render_parts.push_back(&hair);
  if (g_character.coat.has_value()) {
    const auto &coat =
        character_game_instance::avatar_data.at(g_character.coat->id);
    render_parts.push_back(&coat);
  }
  if (g_character.cap.has_value()) {
    const auto &cap =
        character_game_instance::avatar_data.at(g_character.cap->id);
    render_parts.push_back(&cap);
  }
  if (g_character.weapon.has_value()) {
    const auto &weapon =
        character_game_instance::avatar_data.at(g_character.weapon->id);
    render_parts.push_back(&weapon);
  }
  if (g_character.pant.has_value()) {
    const auto &pant =
        character_game_instance::avatar_data.at(g_character.pant->id);
    render_parts.push_back(&pant);
  }
  const std::u16string action = u"stand1";
  const auto face = character_game_instance::face_data.at(g_character.face.id)
                        .data.at(u"default");
  render_parts.push_back(&face);

  for (const auto part : render_parts) {
    if (!part->data.contains(action)) {
      continue;
    }
    const auto &pts = part->data.at(action)[0];
    for (auto &pt : pts) {
      renders.insert({pt.z, &pt});
    }
  }
  for (const auto &index : character_game_instance::smap) {
    if (renders.contains(index)) {
      auto [first, last] = renders.equal_range(index);
      for (auto it = first; it != last; ++it) {
        const character_avatar *avatar = it->second;
        // 处理 avatar
        auto texture = avatar->texture;
        auto origin = avatar->origin;
        SDL_FRect pos_rect = {
            .x = g_character.pos.x + avatar->pos.x - origin.x - camera.x,
            .y = g_character.pos.y + avatar->pos.y - origin.y - camera.y,
            .w = static_cast<float>(texture->w),
            .h = static_cast<float>(texture->h),
        };
        SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
      }
    }
  }
  return true;
}