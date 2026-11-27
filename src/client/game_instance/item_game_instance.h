#pragma once

#include "src/client/game/game_character.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_item_buff.h"

#include <wz/Node.h>

#include <cstdint>
#include <memory>
#include <string>

namespace sdlms {

class ItemGameInstance {
 public:
  // 095 CField::BlowWeather 读取的天气道具参数（info 下的字段）。
  struct ItemWeatherInfo {
    int type = 0;        // 1/2=普通天气 3=爆竹 4=新年
    int float_type = 0;  // FloatNotice 底板序号（缺省 0）
    int direction = 0;
    int speed = 0;
    int is_bgm = 0;   // isBgmOrEffect：1=BGM 0=音效
    int repeat = 0;   // 音效是否循环
    std::u16string path;  // Map/MapHelper.img/weather/...
    std::u16string bgm;   // 天气 BGM/音效路径
  };

  static inline std::vector<GameItemBuff> buff_;

  static bool CheckItem(const std::u16string& id);

  static std::u16string LoadItemText(const std::u16string& id,
                                     const std::u16string& val);
  static std::u16string LoadItemType(const std::u16string& id);
  static wz::Node* LoadItemInfo(const std::u16string& id, uint32_t num);
  // 天气道具（095 现金道具 slot 512 -> type 16）：info/floatType 选 FloatNotice
  // 底板，没有该字段返回 0。
  static int LoadItemFloatType(const std::u16string& id);
  // 读取完整天气参数；不是天气道具返回 false。
  static bool LoadItemWeather(const std::u16string& id, ItemWeatherInfo& out);
  static int LoadSlotMax(const std::u16string& id);
  static std::polymorphic<GameItem> LoadItem(const std::u16string& id,
                                             uint32_t num);
  static int LoadItemNum(std::polymorphic<GameItem>& itm);
  static void AddItemNum(std::polymorphic<GameItem>& item, int num);
  static void DecItemNum(std::polymorphic<GameItem>& item, int num);
  // 按 id 扣除一件现金道具（天气道具使用后消耗）。
  static void DecItemById(const std::u16string& id);
  static void SetItemNum(std::polymorphic<GameItem>& itm, int num);

  static void UseMorphItem(const std::u16string& id, GameCharacter& g);
  static void UnuseMorphItem(const std::u16string& id, GameCharacter& g);
  static bool UseMobItem(std::polymorphic<GameItem>& itm);
  static bool UseBuffItem(std::polymorphic<GameItem>& itm);
  static void UnuseBuffItem(const std::u16string& id);
  static bool UseConsumeItem(const std::u16string& id);
  static bool UseConsumeItem(std::polymorphic<GameItem>& itm);
  // 所有使用物品的入口共用 1 秒冷却。
  static inline uint64_t item_use_cooldown_ = 0;
  static void UseItem(std::polymorphic<GameItem>& itm);

  // 骑宠道具 id（095：Hog/猪猪 01902000），开发便利：背包保底一张。
  static void EnsureMountItem();
};

}  // namespace sdlms
