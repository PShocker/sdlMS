#include "src/client/game_instance/weather_game_instance.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_render.h>
#include <wz/Property.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <utility>

namespace sdlms {

namespace {

bool IsNumberKey(const std::u16string& key) {
  return !key.empty() && std::ranges::all_of(key, [](char16_t c) {
    return c >= u'0' && c <= u'9';
  });
}

int LoadInt(wz::Node* node, const char16_t* name, int fallback) {
  auto* child = node->get_child(name);
  if (child == nullptr || child->type != wz::Type::Int) {
    return fallback;
  }
  return static_cast<wz::Property<int32_t>*>(child)->get();
}

int ParseNumberKey(const std::u16string& key) {
  int value = 0;
  for (auto c : key) {
    value = value * 10 + (c - u'0');
  }
  return value;
}

// 按数字编号升序取动画帧（flat_map 是字典序，10 会排在 2 前面）。
std::vector<wz::Node*> LoadFrames(wz::Node* node) {
  std::vector<std::pair<int, wz::Node*>> tmp;
  for (auto [key, values] : node->children) {
    if (!IsNumberKey(key)) {
      continue;
    }
    auto index = ParseNumberKey(key);
    for (auto* value : values) {
      tmp.emplace_back(index, value);
    }
  }
  std::ranges::sort(tmp, [](const auto& a, const auto& b) {
    return a.first < b.first;
  });
  std::vector<wz::Node*> frames;
  frames.reserve(tmp.size());
  for (auto& [index, value] : tmp) {
    frames.push_back(value);
  }
  return frames;
}

std::u16string StripMapPrefix(std::u16string path) {
  if (path.starts_with(u"Map/")) {
    path = path.substr(4);
  }
  return path;
}

// 095 TransientLayer_Weather：粒子从屏幕边缘飘入，坐标以屏幕中心为基准。
// 本数据包的天气道具没有 info/direction / info/speed，按 direction=0（下落）
// 与 speed=0 处理；speed 为 0 时用 10s 铺满一屏代替 095 的 50s 理论值。
constexpr float kDefaultFallMs = 10000.0f;
constexpr int kMaxParticles = 160;
constexpr int kMaxBursts = 240;
constexpr int kFireCrackerDuration = 30000;
constexpr int kNewYearDuration = 30000;

SDL_FPoint RandomBurstPos() {
  auto sw = CameraGameInstance::camera_.w;
  auto sh = CameraGameInstance::camera_.h;
  // 095 用屏幕矩形的 5/6 作为生成区域（四周各留 1/12 边距）。
  auto x0 = sw / 12;
  auto x1 = sw - sw / 12;
  auto y0 = sh / 12;
  auto y1 = sh - sh / 12;
  auto w = std::max(1, static_cast<int>(x1 - x0));
  auto h = std::max(1, static_cast<int>(y1 - y0));
  return SDL_FPoint{
      x0 + static_cast<float>(std::rand() % w),
      y0 + static_cast<float>(std::rand() % h),
  };
}

}  // namespace

bool WeatherGameInstance::Exists() {
  return !particles_.empty() || !bursts_.empty() || !emitters_.empty();
}

void WeatherGameInstance::Clear(bool restore_bgm) {
  particles_.clear();
  bursts_.clear();
  emitters_.clear();
  expire_ = 0;
  // 095 CField::RestoreBGM：天气 BGM 结束后恢复地图 BGM。
  if (bgm_started_ && restore_bgm) {
    AudioGameInstance::LoadBackgrndAudio(
        MapInfoGameInstance::LoadBgm(SceneSystemInstance::map_id_));
  }
  bgm_started_ = false;
}

void WeatherGameInstance::StartAudio(const std::u16string& path, int is_bgm,
                                     int repeat) {
  if (path.empty()) {
    return;
  }
  if (is_bgm != 0) {
    // 095 CSoundMan::PlayBGM：天气音乐替换当前 BGM，结束时恢复。
    AudioGameInstance::LoadBackgrndAudio(path);
    bgm_started_ = true;
  } else {
    // 095 CSoundMan::PlaySE；repeat 循环需要音频层句柄支持，本数据包无该字段。
    (void)repeat;
    AudioGameInstance::PlayNow(path);
  }
}

void WeatherGameInstance::Start(const std::u16string& path, int direction,
                                int speed, uint64_t duration) {
  Clear();

  auto* node = WzResource::map_->find(StripMapPrefix(path));
  if (node == nullptr) {
    return;
  }

  std::vector<wz::Node*> sprites;
  for (auto [key, values] : node->children) {
    if (!IsNumberKey(key)) {
      continue;
    }
    for (auto* value : values) {
      sprites.push_back(value);
    }
  }
  if (sprites.empty()) {
    return;
  }

  auto count = LoadInt(node, u"count", 100);
  auto random = LoadInt(node, u"random", 0) != 0;
  auto noresize = LoadInt(node, u"noresize", 0) != 0;

  // 095：random=1 时粒子数为 count 的 30%~55%。
  auto n = random ? (std::rand() % 25 + 30) * count / 100 : count;
  n = std::clamp(n, 1, kMaxParticles);

  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  if (screen_w <= 0 || screen_h <= 0) {
    return;
  }
  auto sw = static_cast<float>(screen_w);
  auto sh = static_cast<float>(screen_h);
  auto screen_wi = static_cast<int>(screen_w);
  auto screen_hi = static_cast<int>(screen_h);

  // 095 nSpeeda = 50000 - 8000 * nSpeed（ms），移动距离为半个屏幕。
  auto duration_ms = speed > 0 ? static_cast<float>(50000 - 8000 * speed)
                               : kDefaultFallMs;
  duration_ms = std::max(duration_ms, 1000.0f);

  particles_.reserve(n);
  for (int i = 0; i < n; ++i) {
    GameWeatherParticle p;
    p.sprite = sprites[std::rand() % sprites.size()];
    if (auto* origin = p.sprite->get_child(u"origin"); origin != nullptr) {
      p.origin = WzResource::LoadFpoint(origin);
    }
    if (!noresize) {
      // 095：nSize = rand()%4 * rand()%4 + 15，按画布宽度换算成缩放。
      auto size = std::rand() % 4 * (std::rand() % 4) + 15;
      auto texture = WzResource::LoadTexture(p.sprite);
      if (texture != nullptr && texture->w > 0) {
        p.scale = static_cast<float>(texture->w + size) / texture->w;
      }
    }
    if (p.scale <= 0) {
      p.scale = 1.0f;
    }

    p.base_x = static_cast<float>(std::rand() % screen_wi);
    p.pos.x = p.base_x;
    p.pos.y = static_cast<float>(std::rand() % screen_hi);
    p.sway_phase = static_cast<float>(std::rand() % 628) / 100.0f;

    switch (direction) {
      case 1: {  // 向上
        p.vy = -(sh / 2) / duration_ms;
        p.vx = static_cast<float>(std::rand() % 201 - 100) / duration_ms;
        break;
      }
      case 2: {  // 向左
        p.vx = -(sw / 2) / duration_ms;
        p.vy = static_cast<float>(std::rand() % 201 - 100) / duration_ms;
        break;
      }
      case 3: {  // 向右
        p.vx = (sw / 2) / duration_ms;
        p.vy = static_cast<float>(std::rand() % 201 - 100) / duration_ms;
        break;
      }
      case 6: {  // 原地
        p.vx = 0;
        p.vy = 0;
        break;
      }
      default: {  // 0（下落）/ 4 / 5 兜底
        p.vy = (sh / 2) / duration_ms;
        p.vx = static_cast<float>(std::rand() % 201 - 100) / duration_ms;
        break;
      }
    }

    // 下落/上升时横向轻微摆动，雪花不会直线上落。
    if (p.vy != 0 && direction != 4 && direction != 5) {
      p.sway_amp = static_cast<float>(std::rand() % 17 + 8);
      p.sway_speed = 0.0015f + static_cast<float>(std::rand() % 20) / 10000.0f;
    }
    // 095：粒子初始随机角度，旋转到 0（2000ms）；这里用缓慢自转近似。
    p.angle = static_cast<float>(std::rand() % 360);
    p.spin = (std::rand() % 2 == 0 ? 1.0f : -1.0f) *
             (0.002f + static_cast<float>(std::rand() % 6) / 1000.0f);

    particles_.push_back(p);
  }

  expire_ = duration == 0 ? 0 : Window::dt_now_ + duration;
}

void WeatherGameInstance::StartFireCracker(const std::u16string& path) {
  Clear();

  auto rel = StripMapPrefix(path);
  // 095 TransientLayer_FireCracker：%s0 ~ %s4，参数 (间隔, 数量) 固定。
  static constexpr int kInterval[5] = {200, 1000, 500, 300, 200};
  static constexpr int kCount[5] = {1, 5, 3, 2, 1};

  auto now = Window::dt_now_;
  for (int i = 0; i < 5; ++i) {
    auto name = rel + static_cast<char16_t>(u'0' + i);
    auto* node = WzResource::map_->find(name);
    if (node == nullptr) {
      continue;
    }
    auto frames = LoadFrames(node);
    if (frames.empty()) {
      continue;
    }
    GameWeatherEmitter emitter;
    emitter.frames = std::move(frames);
    emitter.interval = kInterval[i];
    emitter.count = kCount[i];
    emitter.frame_delay = 200;
    emitter.next = now;
    emitters_.push_back(std::move(emitter));
  }

  if (emitters_.empty()) {
    return;
  }
  expire_ = now + kFireCrackerDuration;
}

void WeatherGameInstance::StartNewYear(const std::u16string& path) {
  Clear();

  auto rel = StripMapPrefix(path);
  // 095 TransientLayer_NewYear：动画在 path/0，每 2000ms 生成 3 个，帧间隔 100ms。
  auto* node = WzResource::map_->find(rel + u"/0");
  if (node == nullptr) {
    return;
  }
  auto frames = LoadFrames(node);
  if (frames.empty()) {
    return;
  }

  GameWeatherEmitter emitter;
  emitter.frames = std::move(frames);
  emitter.interval = 2000;
  emitter.count = 3;
  emitter.frame_delay = 100;
  emitter.next = Window::dt_now_;
  emitters_.push_back(std::move(emitter));

  expire_ = Window::dt_now_ + kNewYearDuration;
}

bool WeatherGameInstance::Run() {
  if (!Exists()) {
    return true;
  }
  if (expire_ != 0 && Window::dt_now_ > expire_) {
    Clear();
    return true;
  }

  auto now = Window::dt_now_;
  auto dt = static_cast<float>(Window::delta_time_);
  auto t = static_cast<float>(now);
  auto sw = static_cast<float>(CameraGameInstance::camera_.w);
  auto sh = static_cast<float>(CameraGameInstance::camera_.h);

  // 粒子层（普通天气）。
  for (auto& p : particles_) {
    p.base_x += p.vx * dt;
    p.pos.y += p.vy * dt;
    p.angle += p.spin * dt;

    if (p.sway_amp != 0) {
      p.pos.x = p.base_x + std::sin(t * p.sway_speed + p.sway_phase) * p.sway_amp;
    } else {
      p.pos.x = p.base_x;
    }

    // 循环补位：飘出屏幕后从另一侧重新进入，保证 duration 内画面持续。
    if (p.vy > 0 && p.pos.y > sh + 48) {
      p.pos.y = -48.0f - static_cast<float>(std::rand() % 64);
      p.base_x = static_cast<float>(std::rand() % static_cast<int>(sw));
      p.pos.x = p.base_x;
    } else if (p.vy < 0 && p.pos.y < -48) {
      p.pos.y = sh + 48.0f + static_cast<float>(std::rand() % 64);
      p.base_x = static_cast<float>(std::rand() % static_cast<int>(sw));
      p.pos.x = p.base_x;
    }
    if (p.base_x < -64) {
      p.base_x = sw + 48;
    } else if (p.base_x > sw + 64) {
      p.base_x = -48;
    }
  }

  // 发射器：按间隔在屏幕内随机位置生成爆竹/新年动画。
  for (auto& emitter : emitters_) {
    if (static_cast<int>(bursts_.size()) >= kMaxBursts) {
      break;
    }
    if (now < emitter.next) {
      continue;
    }
    for (int i = 0; i < emitter.count; ++i) {
      GameWeatherBurst burst;
      burst.frames = emitter.frames;
      burst.pos = RandomBurstPos();
      burst.frame = 0;
      burst.frame_delay = emitter.frame_delay;
      burst.frame_time = now + emitter.frame_delay;
      bursts_.push_back(std::move(burst));
    }
    emitter.next = now + emitter.interval;
  }

  // 帧推进：播完即移除。
  for (auto it = bursts_.begin(); it != bursts_.end();) {
    while (it->frame < it->frames.size() && now >= it->frame_time) {
      ++it->frame;
      it->frame_time += it->frame_delay;
    }
    if (it->frame >= it->frames.size()) {
      it = bursts_.erase(it);
    } else {
      ++it;
    }
  }
  return true;
}

bool WeatherGameInstance::Render() {
  for (const auto& p : particles_) {
    if (p.sprite == nullptr) {
      continue;
    }
    auto* texture = WzResource::LoadTexture(p.sprite);
    if (texture == nullptr) {
      continue;
    }
    SDL_FRect dst{
        p.pos.x - p.origin.x * p.scale,
        p.pos.y - p.origin.y * p.scale,
        static_cast<float>(texture->w) * p.scale,
        static_cast<float>(texture->h) * p.scale,
    };
    SDL_FPoint center{dst.w / 2, dst.h / 2};
    SDL_RenderTextureRotated(Window::renderer_, texture, nullptr, &dst, p.angle,
                             &center, SDL_FLIP_NONE);
  }
  for (const auto& b : bursts_) {
    if (b.frames.empty() || b.frame >= b.frames.size()) {
      continue;
    }
    auto* node = b.frames[b.frame];
    if (node == nullptr) {
      continue;
    }
    auto* texture = WzResource::LoadTexture(node);
    if (texture == nullptr) {
      continue;
    }
    SDL_FPoint origin{};
    if (auto* origin_node = node->get_child(u"origin");
        origin_node != nullptr) {
      origin = WzResource::LoadFpoint(origin_node);
    }
    SDL_FRect dst{
        b.pos.x - origin.x,
        b.pos.y - origin.y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &dst);
  }
  return true;
}

}  // namespace sdlms
