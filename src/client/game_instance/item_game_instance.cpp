#include "item_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <ranges>
#include <string>

std::u16string item_game_instance::load_item_name(game_item &item) {
  static auto str_node = wz_resource::string->find(u"Etc.img/Etc");
  auto view =
      item.id | std::views::drop_while([](char16_t c) { return c == u'0'; });
  std::u16string result(view.begin(), view.end());

  auto str = str_node->get_child(result)->get_child(u"name");
  return static_cast<wz::Property<std::u16string> *>(str)->get();
}