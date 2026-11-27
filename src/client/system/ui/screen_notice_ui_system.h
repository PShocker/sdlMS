#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace sdlms {

// 对应 095 CFloatNotice（UI/UIWindow.img/FloatNotice/%d）：
// 屏幕顶部居中的浮动公告条。底图由左端片 0、中段平铺片 1、右端片 2 三段拼成，
// 中段片数按文字宽度计算；文字纵向偏移取节点的 y，颜色优先取 fontColor。
class ScreenNoticeUiSystem {
 private:
  struct Notice {
    std::u16string text;
    int type = 0;
    uint64_t expire = 0;  // 0 表示不自动消失
  };

  static inline std::optional<Notice> float_notice_;
  static inline std::optional<Notice> item_msg_;

  static void RenderNotice(const Notice& notice);

 public:
  // 095 CUIStatusBar::FloatNotice：先清掉道具消息，再创建浮动公告。
  // duration 为 0 时保持到显式删除或切图。
  static void FloatNotice(const std::u16string& text, int type,
                          uint64_t duration);
  static void DeleteFloatNotice();

  // 095 CUIStatusBar::SetItemMsg：已有浮动公告/道具消息时拒绝创建，
  // 返回是否创建成功。
  static bool SetItemMsg(const std::u16string& text, int type,
                         uint64_t duration);
  static void DeleteItemMsg();

  // 095 CWvsContext::OnEnterField：切图时清除全部浮动公告。
  static void Clear();

  static bool Render();
};

}  // namespace sdlms
