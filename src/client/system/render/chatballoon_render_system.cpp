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
#include <wz/Types.h>
#include <wz/Wz.h>

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

// 九宫格的一块素材：纹理由 WzResource 解析 UOL / _outlink，
// origin 是这块素材贴在内容区四角时的锚点。
struct Piece {
  SDL_Texture* texture = nullptr;
  SDL_FPoint origin = {0.0f, 0.0f};
};

wz::Node* ResolveUol(wz::Node* node) {
  while (node != nullptr && node->type == wz::Type::UOL) {
    node = static_cast<wz::Property<wz::WzUOL>*>(node)->get_uol();
  }
  return node;
}

Piece LoadPiece(wz::Node* node, const char16_t* name) {
  Piece piece;
  if (node == nullptr) {
    return piece;
  }
  auto* child = node->get_child(name);
  if (child == nullptr) {
    return piece;
  }
  piece.texture = WzResource::LoadTexture(child);
  auto* real = ResolveUol(child);
  if (real == nullptr) {
    return piece;
  }
  if (auto* origin = real->get_child(u"origin")) {
    auto v = static_cast<wz::Property<wz::WzVec2D>*>(origin)->get();
    piece.origin = {static_cast<float>(v.x), static_cast<float>(v.y)};
  }
  return piece;
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

  Piece c = LoadPiece(node, u"c");
  Piece n = LoadPiece(node, u"n");
  Piece s = LoadPiece(node, u"s");
  Piece w = LoadPiece(node, u"w");
  Piece e = LoadPiece(node, u"e");
  Piece nw = LoadPiece(node, u"nw");
  Piece ne = LoadPiece(node, u"ne");
  Piece sw = LoadPiece(node, u"sw");
  Piece se = LoadPiece(node, u"se");
  Piece arrow = LoadPiece(node, u"arrow");
  if (c.texture == nullptr || nw.texture == nullptr || ne.texture == nullptr ||
      sw.texture == nullptr || se.texture == nullptr ||
      arrow.texture == nullptr) {
    return std::nullopt;
  }
  // 直边缺失时用中心块顶替
  if (n.texture == nullptr) {
    n = c;
  }
  if (s.texture == nullptr) {
    s = c;
  }
  if (w.texture == nullptr) {
    w = c;
  }
  if (e.texture == nullptr) {
    e = c;
  }

  // 换行宽度：请求宽度向上取整到中心块宽度，右边不留残块
  const int inner_w_int = CeilToMultiple(
      static_cast<int>(std::max(g_chatballoon.w, kMinInnerWidth)), c.texture->w);
  const float inner_w = static_cast<float>(inner_w_int);

  Freetype::LoadSize(g_chatballoon.size);
  // 排版和真正绘制必须使用同样的字重，否则换行宽度会不同
  Freetype::LoadBold(false);
  const float block_h = Freetype::LoadCh(
      g_chatballoon.text, inner_w, kLineSpacing);
  // 中心区域取整到整块高度，文字在区域内垂直居中
  int inner_h_int =
      CeilToMultiple(static_cast<int>(std::ceil(block_h)), c.texture->h);
  if (inner_h_int < c.texture->h) {
    inner_h_int = c.texture->h;
  }
  const float inner_h = static_cast<float>(inner_h_int);

  // 四边 / 四角相对内容区的伸出量。素材并不总是正方形：
  // 例如 25 号气泡的 nw 是 25x26、ne 是 16x15，必须各自按 origin 摆放。
  const float left = std::max({nw.origin.x, sw.origin.x, w.origin.x});
  const float right =
      std::max({static_cast<float>(ne.texture->w) - ne.origin.x,
                static_cast<float>(se.texture->w) - se.origin.x,
                static_cast<float>(e.texture->w) - e.origin.x});
  const float top = std::max({nw.origin.y, ne.origin.y, n.origin.y});
  const float bottom =
      std::max({static_cast<float>(sw.texture->h) - sw.origin.y,
                static_cast<float>(se.texture->h) - se.origin.y,
                static_cast<float>(s.texture->h) - s.origin.y});

  const float box_w = inner_w + left + right;

  // 尾巴尖端对准 base，尾巴底部就是整个气泡的最低点；
  // 内容区底边（南边素材的锚点）由尾巴高度反推。
  const float tip_y = base.y - kTipGap;
  const float arrow_h = static_cast<float>(arrow.texture->h);
  const float iy1 = tip_y + arrow.origin.y - arrow_h;
  const float iy0 = iy1 - inner_h;
  const float box_top = iy0 - top;
  const float box_bottom = std::max(tip_y, iy1 + bottom);

  // 整块气泡以 base 为中心（和 v95 把 layer 居中到角色坐标一致）
  const float box_left = base.x - box_w / 2.0f;

  SDL_FRect world{box_left, box_top, box_w, box_bottom - box_top};
  auto& camera = CameraGameInstance::camera_;
  if (!SDL_HasRectIntersectionFloat(&world, &camera)) {
    return std::nullopt;
  }

  // 转到屏幕坐标。所有素材都先摆到以气泡左上角为原点的局部坐标，
  // 最后统一减去相机偏移。
  const float ox = box_left - camera.x;
  const float oy = box_top - camera.y;
  const float ix0 = left;
  const float iy0_local = top;
  const float ix1 = ix0 + inner_w;
  const float iy1_local = iy0_local + inner_h;

  // 先铺内容区，再让四边 / 四角盖住接缝（部分气泡的角块会向内容区伸出）
  DrawTiled(c.texture,
            SDL_FRect{ox + ix0, oy + iy0_local, inner_w, inner_h});

  DrawTiled(n.texture, SDL_FRect{ox + ix0, oy + iy0_local - n.origin.y,
                                 inner_w, static_cast<float>(n.texture->h)});
  DrawTiled(w.texture,
            SDL_FRect{ox + ix0 - w.origin.x, oy + iy0_local,
                      static_cast<float>(w.texture->w), inner_h});
  DrawTiled(e.texture,
            SDL_FRect{ox + ix1 - e.origin.x, oy + iy0_local,
                      static_cast<float>(e.texture->w), inner_h});

  // 尾巴放在内容区底部；南边素材在尾巴两侧分段绘制，避免半透明像素
  // 叠加在尾巴自带的底边上（25 号气泡的 arrow 上半部分就是一段底边）。
  const float arrow_x =
      base.x - box_left - static_cast<float>(arrow.texture->w) / 2.0f;
  const float s_y = oy + iy1_local - s.origin.y;
  const float s_left_w = arrow_x + arrow.origin.x - ix0;
  if (s_left_w > 0.0f) {
    DrawTiled(s.texture, SDL_FRect{ox + ix0, s_y, s_left_w,
                                   static_cast<float>(s.texture->h)});
  }
  const float s_right_x = arrow_x + static_cast<float>(arrow.texture->w);
  if (s_right_x < ix1) {
    DrawTiled(s.texture, SDL_FRect{ox + s_right_x, s_y, ix1 - s_right_x,
                                   static_cast<float>(s.texture->h)});
  }

  DrawPiece(nw.texture,
            SDL_FRect{ox + ix0 - nw.origin.x, oy + iy0_local - nw.origin.y,
                      static_cast<float>(nw.texture->w),
                      static_cast<float>(nw.texture->h)});
  DrawPiece(ne.texture,
            SDL_FRect{ox + ix1 - ne.origin.x, oy + iy0_local - ne.origin.y,
                      static_cast<float>(ne.texture->w),
                      static_cast<float>(ne.texture->h)});
  DrawPiece(sw.texture,
            SDL_FRect{ox + ix0 - sw.origin.x, oy + iy1_local - sw.origin.y,
                      static_cast<float>(sw.texture->w),
                      static_cast<float>(sw.texture->h)});
  DrawPiece(se.texture,
            SDL_FRect{ox + ix1 - se.origin.x, oy + iy1_local - se.origin.y,
                      static_cast<float>(se.texture->w),
                      static_cast<float>(se.texture->h)});

  DrawPiece(arrow.texture,
            SDL_FRect{ox + arrow_x, oy + iy1_local - arrow.origin.y,
                      static_cast<float>(arrow.texture->w),
                      static_cast<float>(arrow.texture->h)});

  std::optional<int32_t> clr;
  if (auto* clr_node = node->get_child(u"clr")) {
    clr = static_cast<wz::Property<int32_t>*>(clr_node)->get();
  }

  // 文字区：内容区，垂直居中
  const float text_y =
      oy + iy0_local + std::max(0.0f, (inner_h - block_h) / 2.0f);
  return BackgrndPad{
      SDL_FRect{ox, oy, box_w, box_bottom - box_top},
      SDL_FRect{ox + ix0, text_y, inner_w, inner_h},
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
