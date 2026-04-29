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
  render_parts.push_back(&head);
  render_parts.push_back(&body);
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

  for (const auto part : render_parts) {
    const auto &pts = part->data.at(u"stand1")[0];
    for (auto &pt : pts) {
      renders.insert({pt.z, &pt});
    }
  }
  for (const auto &[k, v] : renders) {
    auto texture = v->texture;
    auto origin = v->origin;
    SDL_FRect pos_rect = {
        .x = g_character.pos.x + v->pos.x - origin.x - camera.x,
        .y = g_character.pos.y + v->pos.y - origin.y - camera.y,
        .w = static_cast<float>(texture->w),
        .h = static_cast<float>(texture->h),
    };
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }
  return true;
}