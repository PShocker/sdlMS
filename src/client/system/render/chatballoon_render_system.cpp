#include "src/client/system/render/chatballoon_render_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>

namespace sdlms {

namespace {

// 尾巴尖端与头顶的间距（v95 CChatBalloon::AdjustCoordY 里是 -nHeight - 5）
constexpr float kTipGap = 5.0f;
// 行高倍数，必须和 Freetype::LoadCh / DrawCstr 用的保持一致
constexpr float kLineSpacing = 1.3f;
// 文字区最小宽度，避免请求宽度过小时挤成一列
constexpr float kMinInnerWidth = 40.0f;

// 取九宫格的一块素材；取不到时返回 nullptr
SDL_Texture* LoadPiece(wz::Node* node, const char16_t* name) {
  if (node == nullptr) {
    return nullptr;
  }
  auto* child = node->get_child(name);
  if (child == nullptr) {
    return nullptr;
  }
  return WzResource::LoadTexture(child);
}

void DrawPiece(SDL_Texture* texture, const SDL_FRect& dst) {
  if (texture == nullptr || dst.w <= 0.0f || dst.h <= 0.0f) {
    return;
  }
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &dst);
}

// 平铺一块素材填满 dst
void DrawTiled(SDL_Texture* texture, const SDL_FRect& dst) {
  if (texture == nullptr || dst.w <= 0.0f || dst.h <= 0.0f) {
    return;
  }
  SDL_RenderTextureTiled(Window::renderer_, texture, nullptr, 1.0f, &dst);
}

}  // namespace

int ChatballoonRenderSystem::CeilToMultiple(int value, int tile) {
  if (tile <= 0) {
    return value;
  }
  if (value <= 0) {
    return tile;
  }
  return tile * ((value + tile - 1) / tile);
}

std::optional<ChatballoonRenderSystem::BackgrndPad>
ChatballoonRenderSystem::RenderBackgrnd(GameChatballoon& g_chatballoon,
                                        SDL_FPoint base) {
  auto node = WzResource::ui_->find(u"ChatBalloon.img/" + g_chatballoon.path);
  if (node == nullptr) {
    // 编号不存在时退回 0 号气泡，避免整条消息不显示
    node = WzResource::ui_->find(u"ChatBalloon.img/0");
  }
  if (node == nullptr) {
    return std::nullopt;
  }

  SDL_Texture* c = LoadPiece(node, u"c");
  SDL_Texture* n = LoadPiece(node, u"n");
  SDL_Texture* s = LoadPiece(node, u"s");
  SDL_Texture* w = LoadPiece(node, u"w");
  SDL_Texture* e = LoadPiece(node, u"e");
  SDL_Texture* nw = LoadPiece(node, u"nw");
  SDL_Texture* ne = LoadPiece(node, u"ne");
  SDL_Texture* sw = LoadPiece(node, u"sw");
  SDL_Texture* se = LoadPiece(node, u"se");
  SDL_Texture* arrow = LoadPiece(node, u"arrow");
  if (c == nullptr || nw == nullptr || ne == nullptr || sw == nullptr ||
      se == nullptr || arrow == nullptr) {
    return std::nullopt;
  }
  // 直边缺失时用中心块顶替
  if (n == nullptr) {
    n = c;
  }
  if (s == nullptr) {
    s = c;
  }
  if (w == nullptr) {
    w = c;
  }
  if (e == nullptr) {
    e = c;
  }

  const float corner_w = static_cast<float>(nw->w);
  const float corner_h = static_cast<float>(nw->h);
  const float arrow_w = static_cast<float>(arrow->w);
  const float arrow_h = static_cast<float>(arrow->h);

  // 换行宽度：请求宽度向上取整到中心块宽度，右边不留残块
  const int inner_w = CeilToMultiple(
      static_cast<int>(std::max(g_chatballoon.w, kMinInnerWidth)), c->w);

  Freetype::LoadSize(g_chatballoon.size);
  const float block_h = Freetype::LoadCh(
      g_chatballoon.text, static_cast<float>(inner_w), kLineSpacing);
  // 中心区域取整到整块高度，文字在区域内垂直居中
  int inner_h = CeilToMultiple(static_cast<int>(std::ceil(block_h)), c->h);
  if (inner_h < c->h) {
    inner_h = c->h;
  }

  const float box_w = static_cast<float>(inner_w) + corner_w * 2.0f;
  const float box_h = static_cast<float>(inner_h) + corner_h * 2.0f;

  // 尾巴尖端对准 base，尾巴上端压在底部边框下面
  const float tip_y = base.y - kTipGap;
  const float box_bottom = tip_y - std::max(0.0f, arrow_h - corner_h);
  const float box_top = box_bottom - box_h;
  const float box_left = base.x - box_w / 2.0f;

  SDL_FRect world{box_left, box_top, box_w, tip_y - box_top};
  auto& camera = CameraGameInstance::camera_;
  if (!SDL_HasRectIntersectionFloat(&world, &camera)) {
    return std::nullopt;
  }

  // 转到屏幕坐标
  const float x = box_left - camera.x;
  const float y = box_top - camera.y;
  const float right = x + box_w;
  const float bottom = y + box_h;

  // 四个角
  DrawPiece(nw, SDL_FRect{x, y, corner_w, corner_h});
  DrawPiece(ne, SDL_FRect{right - ne->w, y, static_cast<float>(ne->w),
                          static_cast<float>(ne->h)});
  DrawPiece(sw, SDL_FRect{x, bottom - sw->h, static_cast<float>(sw->w),
                          static_cast<float>(sw->h)});
  DrawPiece(se,
            SDL_FRect{right - se->w, bottom - se->h, static_cast<float>(se->w),
                      static_cast<float>(se->h)});

  // 中心
  DrawTiled(c, SDL_FRect{x + nw->w, y + n->h, right - se->w - (x + nw->w),
                         bottom - sw->h - (y + n->h)});
  // 上下边
  DrawTiled(n, SDL_FRect{x + nw->w, y, right - ne->w - (x + nw->w),
                         static_cast<float>(n->h)});
  // 左右边
  DrawTiled(w, SDL_FRect{x, y + nw->h, static_cast<float>(w->w),
                         bottom - sw->h - (y + nw->h)});
  DrawTiled(e, SDL_FRect{right - e->w, y + ne->h, static_cast<float>(e->w),
                         bottom - se->h - (y + ne->h)});

  DrawTiled(s, SDL_FRect{x + sw->w, bottom - s->h, right - se->w - (x + sw->w),
                         static_cast<float>(s->h)});

  // 尾巴
  const float arrow_x = x + box_w / 2.0f - arrow_w / 2.0f;
  const float arrow_y = bottom - corner_h;
  DrawPiece(arrow, SDL_FRect{arrow_x, arrow_y, arrow_w, arrow_h});

  std::optional<int32_t> clr;
  if (auto* clr_node = node->get_child(u"clr")) {
    clr = static_cast<wz::Property<int32_t>*>(clr_node)->get();
  }

  // 文字区：内边距去掉四条边框，垂直居中
  const float text_x = x + nw->w;
  const float text_y =
      y + n->h + std::max(0.0f, (static_cast<float>(inner_h) - block_h) / 2.0f);
  const float text_w = right - se->w - text_x;

  return BackgrndPad{
      SDL_FRect{x, y, box_w, tip_y - box_top},
      SDL_FRect{text_x, text_y, text_w, static_cast<float>(inner_h)},
      clr,
  };
}

bool ChatballoonRenderSystem::Render(GameChatballoon& g_chatballoon,
                                     SDL_FPoint base) {
  auto bacgrnd_pad = RenderBackgrnd(g_chatballoon, base);
  if (!bacgrnd_pad.has_value()) {
    return false;
  }

  uint8_t a = 255;
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  if (g_chatballoon.color.has_value()) {
    a = g_chatballoon.color->a;
    r = g_chatballoon.color->r;
    g = g_chatballoon.color->g;
    b = g_chatballoon.color->b;
  } else if (bacgrnd_pad->color.has_value()) {
    auto clr = bacgrnd_pad->color.value();
    a = (clr >> 24) & 0xFF;
    r = (clr >> 16) & 0xFF;
    g = (clr >> 8) & 0xFF;
    b = clr & 0xFF;
  }
  Freetype::LoadBold(false);
  Freetype::LoadColor(r, g, b, a);
  Freetype::LoadAligned(true);
  Freetype::LoadSize(g_chatballoon.size);
  Freetype::DrawCstr(g_chatballoon.text, bacgrnd_pad->text.x,
                     bacgrnd_pad->text.y, bacgrnd_pad->text.w, kLineSpacing);
  return true;
}

}  // namespace sdlms
