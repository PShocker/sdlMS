#include "src/client/game_instance/map_clock_game_instance.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_render.h>
#include <wz/Property.h>

#include <array>
#include <cmath>
#include <ctime>
#include <string>

namespace sdlms {

namespace {

// 095 CClock：字体素材 Map/Obj/etc.img/clock/fontTime，含 0-9、comma（冒号）、
// am、pm 共 13 张画布。
struct ClockFont {
  bool loaded = false;
  std::array<SDL_Texture*, 10> digits{};
  SDL_Texture* comma = nullptr;
  SDL_Texture* am = nullptr;
  SDL_Texture* pm = nullptr;
};

ClockFont& Fonts() {
  static ClockFont fonts;
  return fonts;
}

void LoadFonts() {
  auto& fonts = Fonts();
  if (fonts.loaded) {
    return;
  }
  fonts.loaded = true;

  auto* node = WzResource::map_->find(u"Obj/etc.img/clock/fontTime");
  if (node == nullptr) {
    return;
  }
  auto load = [node](const char16_t* name) -> SDL_Texture* {
    auto* child = node->get_child(name);
    if (child == nullptr) {
      return nullptr;
    }
    return WzResource::LoadTexture(child);
  };
  for (int i = 0; i < 10; ++i) {
    auto key = std::u16string(1, static_cast<char16_t>(u'0' + i));
    fonts.digits[i] = load(key.c_str());
  }
  fonts.comma = load(u"comma");
  fonts.am = load(u"am");
  fonts.pm = load(u"pm");
}

void Draw(SDL_Texture* texture, float x, float y) {
  if (texture == nullptr) {
    return;
  }
  SDL_FRect dst{
      x,
      y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &dst);
}

}  // namespace

void MapClockGameInstance::Load(uint32_t map_id) {
  rect_ = std::nullopt;
  auto* map_node = WzResource::LoadMapNode(map_id);
  if (map_node == nullptr) {
    return;
  }
  auto* clock = map_node->get_child(u"clock");
  if (clock == nullptr) {
    return;
  }
  auto load_int = [clock](const char16_t* name) {
    auto* node = clock->get_child(name);
    if (node == nullptr || node->type != wz::Type::Int) {
      return 0;
    }
    return static_cast<wz::Property<int32_t>*>(node)->get();
  };
  auto x = load_int(u"x");
  auto y = load_int(u"y");
  auto w = load_int(u"width");
  auto h = load_int(u"height");
  // 095 RestoreClock：x/y/width/height 任一为 0 时不创建时钟。
  if (x == 0 || y == 0 || w == 0 || h == 0) {
    return;
  }
  rect_ = SDL_FRect{
      static_cast<float>(x),
      static_cast<float>(y),
      static_cast<float>(w),
      static_cast<float>(h),
  };
}

void MapClockGameInstance::Clear() {
  rect_ = std::nullopt;
  timer_end_ = 0;
}

void MapClockGameInstance::Enter(uint32_t map_id) {
  // 095 CClock 倒计时：info/timeLimit 为秒数，结束时间点在进图时确定。
  timer_end_ = 0;
  auto limit = MapInfoGameInstance::LoadTimeLimit(map_id);
  if (limit != 0) {
    timer_end_ = Window::dt_now_ + static_cast<uint64_t>(limit) * 1000;
  }
}

bool MapClockGameInstance::RenderTimer() {
  if (timer_end_ == 0) {
    return true;
  }
  auto now = Window::dt_now_;
  auto remain = timer_end_ > now ? timer_end_ - now : 0;
  auto secs = static_cast<int>((remain + 999) / 1000);
  auto mm = secs / 60;
  auto ss = secs % 60;

  LoadFonts();
  auto& fonts = Fonts();
  if (fonts.digits[0] == nullptr || fonts.comma == nullptr) {
    return true;
  }

  static auto back = WzResource::LoadTexture(
      WzResource::map_->find(u"Obj/etc.img/timer/backgrnd"));
  auto bw = back != nullptr ? static_cast<float>(back->w) : 194.0f;
  auto bh = back != nullptr ? static_cast<float>(back->h) : 53.0f;
  auto x = std::floor((CameraGameInstance::camera_.w - bw) / 2);
  auto y = 30.0f;
  if (back != nullptr) {
    SDL_FRect r{x, y, bw, bh};
    SDL_RenderTexture(Window::renderer_, back, nullptr, &r);
  }

  auto digit_w = static_cast<float>(fonts.digits[0]->w);
  auto comma_w = static_cast<float>(fonts.comma->w);
  auto count = mm >= 10 ? 5 : 4;
  auto total = (mm >= 10 ? digit_w : 0.0f) + digit_w + comma_w + digit_w +
               digit_w + 2.0f * static_cast<float>(count - 1);
  auto cx = x + (bw - total) / 2;
  auto cy = y + 12;

  auto draw = [&](SDL_Texture* texture) {
    if (texture == nullptr) {
      return;
    }
    SDL_FRect r{
        cx,
        cy,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &r);
    cx += static_cast<float>(texture->w) + 2;
  };

  // 095 CClock::Draw 计时分支：mm:ss，十分钟位为 0 时不画。
  if (mm >= 10) {
    draw(fonts.digits[mm / 10]);
  }
  draw(fonts.digits[mm % 10]);
  SDL_SetTextureAlphaMod(fonts.comma, (now / 1000) % 2 != 0 ? 64 : 255);
  draw(fonts.comma);
  SDL_SetTextureAlphaMod(fonts.comma, 255);
  draw(fonts.digits[ss / 10]);
  draw(fonts.digits[ss % 10]);
  return true;
}

bool MapClockGameInstance::Render() {
  if (!rect_.has_value()) {
    return true;
  }
  LoadFonts();
  auto& fonts = Fonts();
  if (fonts.digits[0] == nullptr || fonts.comma == nullptr ||
      fonts.am == nullptr || fonts.pm == nullptr) {
    return true;
  }

  auto now = std::time(nullptr);
  std::tm local{};
  localtime_s(&local, &now);

  // 095 CClock 为非倒计时模式时显示 12 小时制当前时间，冒号每秒闪烁。
  auto hour = local.tm_hour % 12;
  if (hour == 0) {
    hour = 12;
  }
  auto minute = local.tm_min;

  auto& camera = CameraGameInstance::camera_;
  auto screen_x = rect_->x - camera.x;
  auto screen_y = rect_->y - camera.y;

  auto digit_w = static_cast<float>(fonts.digits[0]->w);
  auto digit_h = static_cast<float>(fonts.digits[0]->h);
  auto comma_w = static_cast<float>(fonts.comma->w);
  auto am_w = static_cast<float>(fonts.am->w);

  // 095 Draw 的居中公式：am + (comma 间距) + HH + 冒号 + MM，字符间隔 2px。
  auto x = screen_x +
           static_cast<float>(
               (rect_->w -
                (2 * (comma_w + 2 * digit_w) + am_w + 8)) /
               2);
  auto y = screen_y + (rect_->h - digit_h) / 2;

  Draw(local.tm_hour >= 12 ? fonts.pm : fonts.am, x, y);
  x += am_w + comma_w;

  Draw(fonts.digits[hour / 10], x, y);
  x += digit_w + 2;
  Draw(fonts.digits[hour % 10], x, y);
  x += digit_w + 2;

  SDL_SetTextureAlphaMod(fonts.comma, local.tm_sec % 2 != 0 ? 64 : 255);
  Draw(fonts.comma, x, y);
  SDL_SetTextureAlphaMod(fonts.comma, 255);
  x += comma_w + 2;

  Draw(fonts.digits[minute / 10], x, y);
  x += digit_w + 2;
  Draw(fonts.digits[minute % 10], x, y);
  return true;
}

}  // namespace sdlms
