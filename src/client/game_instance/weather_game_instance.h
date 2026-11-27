#pragma once

#include "src/client/game/game_weather.h"

#include <cstdint>
#include <string>
#include <vector>

namespace sdlms {

// 095 CMapLoadable::TransientLayer_Weather / TransientLayer_Clear：
// 天气道具的粒子层。数据在 Map/MapHelper.img/weather/<name>：
//   <n>       粒子画布（random=1 时随机取一张）
//   count     粒子数（random=1 时取 30%~55%）
//   noresize  1=不随机缩放
// 粒子从屏幕边缘飘入、循环补位，duration 到期整层清除。
class WeatherGameInstance {
 public:
  static inline std::vector<GameWeatherParticle> particles_;
  static inline std::vector<GameWeatherBurst> bursts_;
  static inline std::vector<GameWeatherEmitter> emitters_;
  static inline uint64_t expire_ = 0;

  // path 为道具 info/path（带 "Map/" 前缀），speed 来自 info/speed。
  static void Start(const std::u16string& path, int direction, int speed,
                    uint64_t duration);
  // 095 TransientLayer_FireCracker：path 形如 .../weather/squib/squib，
  // 依次播放 squib0..squib4（间隔/数量 200/1、1000/5、500/3、300/2、200/1）。
  static void StartFireCracker(const std::u16string& path);
  // 095 TransientLayer_NewYear：path/0 为帧动画，每 2000ms 生成 3 个。
  static void StartNewYear(const std::u16string& path);
  // 095 BlowWeather 的音频：isBgmOrEffect=1 时作为 BGM 播放，否则作为音效。
  static void StartAudio(const std::u16string& path, int is_bgm, int repeat);
  // restore_bgm=false 用于切图：新地图 BGM 随后由场景加载。
  static void Clear(bool restore_bgm = true);
  static bool Run();
  static bool Render();

  static bool Exists();

 private:
  static inline bool bgm_started_ = false;
};

}  // namespace sdlms
