#include "character_render_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/window/window.h"

bool character_render_system::render(game_character &g_character) {
  auto &camera = camera_game_instance::camera;
  const auto &avatar =
      character_game_instance::avatar_data.at(g_character.body);
  const auto &c = avatar.data.at(u"stand1");
  const auto &as = c.at(0);
  for (const auto &a : as) {
    auto texture = a.texture;
    auto origin = a.origin;
    SDL_FRect pos_rect = {
        .x = g_character.pos.x + a.pos.x - origin.x - camera.x,
        .y = g_character.pos.y + a.pos.y - origin.y - camera.y,
        .w = static_cast<float>(texture->w),
        .h = static_cast<float>(texture->h),
    };
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }
  const auto &head = character_game_instance::avatar_data.at(g_character.head);
  const auto hc = head.data.at(u"stand1");
  const auto &hs = hc.at(0);
  for (const auto &a : hs) {
    auto texture = a.texture;
    auto origin = a.origin;
    SDL_FRect pos_rect = {
        .x = g_character.pos.x + a.pos.x - origin.x - camera.x,
        .y = g_character.pos.y + a.pos.y - origin.y - camera.y,
        .w = static_cast<float>(texture->w),
        .h = static_cast<float>(texture->h),
    };
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }

  return true;
}