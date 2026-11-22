#include "src/client/game_instance/craft_game_instance.h"

#include "src/common/wz/wz_resource.h"

#include <wz/Property.h>

#include <cstdint>
#include <flat_map>


namespace sdlms {

void CraftGameInstance::Load(const CharacterSaveData& cs) {
  datas_ = {};
  for (uint8_t i = 0; i < static_cast<uint8_t>(CraftType::kLength); ++i) {
    datas_[i].type = static_cast<CraftType>(i);
    datas_[i].level = 1;
    datas_[i].exp = 0;
  }
  for (auto& c : cs.craft) {
    CraftData d;
    d.exp = c.exp;
    d.level = c.level;
    d.type = (CraftType)c.type;
    datas_[c.type] = d;
  }
}

void CraftGameInstance::InitRecipe() {
  recipes_.clear();
  auto* root = WzResource::etc_->find(u"CraftRecipe.img");
  if (root == nullptr || root->get_children() == nullptr) {
    return;
  }
  auto read_int = [](wz::Node* node, const char16_t* path, int32_t def) {
    if (node == nullptr) {
      return def;
    }
    auto* child = node->find(path);
    if (child == nullptr || child->type != wz::Type::Int) {
      return def;
    }
    return static_cast<wz::Property<int32_t>*>(child)->get();
  };
  int type_index = 0;
  for (const auto& [type_key, type_nodes] : *root->get_children()) {
    if (type_nodes.empty() || type_nodes[0] == nullptr) {
      continue;
    }
    (void)type_key;
    // CraftRecipe.img 的一级节点顺序就是 CraftType 0..5。
    const CraftType craft_type = static_cast<CraftType>(type_index++);
    for (const auto& [level_key, level_nodes] : *type_nodes[0]->get_children()) {
      if (level_nodes.empty() || level_nodes[0] == nullptr) {
        continue;
      }
      int level = 1;
      try {
        level = std::stoi(std::string(level_key.begin(), level_key.end()));
      } catch (...) {
        level = 1;
      }
      for (const auto& [recipe_key, recipe_nodes] :
           *level_nodes[0]->get_children()) {
        (void)recipe_key;
        if (recipe_nodes.empty() || recipe_nodes[0] == nullptr) {
          continue;
        }
        auto* node = recipe_nodes[0];
        CraftRecipe cr{};
        cr.type = craft_type;
        cr.level = static_cast<uint8_t>(std::clamp(level, 1, 255));
        cr.coolTimeSec = static_cast<uint32_t>(
            std::max(0, read_int(node, u"CoolTimeSec", 0)));
        cr.processTimeMS = static_cast<uint32_t>(
            std::max(0, read_int(node, u"ProcessTimeMS", 0)));
        cr.meso = static_cast<uint32_t>(std::max(0, read_int(node, u"Meso", 0)));
        cr.conditions.reqLevel = static_cast<uint8_t>(std::clamp(
            read_int(node, u"Conditions/SkillCheck/reqSkillLevel", 1), 1, 255));
        cr.conditions.skillID = static_cast<uint32_t>(std::max(
            0, read_int(node, u"Conditions/SkillCheck/SkillID", 0)));
        if (auto* ingredients = node->get_child(u"Ingredients");
            ingredients != nullptr && ingredients->get_children() != nullptr) {
          for (const auto& [ing_key, ing_nodes] : *ingredients->get_children()) {
            (void)ing_key;
            if (ing_nodes.empty() || ing_nodes[0] == nullptr) {
              continue;
            }
            auto* ing = ing_nodes[0];
            CraftInredient ci{};
            ci.item = static_cast<uint32_t>(
                std::max(0, read_int(ing, u"item", 0)));
            ci.count = static_cast<uint8_t>(std::clamp(
                read_int(ing, u"count", 1), 1, 255));
            if (ci.item != 0) {
              cr.ingredients.emplace_back(ci);
            }
          }
        }
        cr.result.item = static_cast<uint32_t>(
            std::max(0, read_int(node, u"Results/item", 0)));
        cr.result.count = static_cast<uint32_t>(
            std::max(1, read_int(node, u"Results/count", 1)));
        cr.result.exp = static_cast<uint32_t>(
            std::max(0, read_int(node, u"Results/exp", 0)));
        if (cr.result.item != 0) {
          recipes_.emplace_back(std::move(cr));
        }
      }
    }
  }
}
std::vector<CraftGameInstance::CraftRecipe> CraftGameInstance::LoadSkillRecipes(
    uint32_t skillID) {
  static std::flat_map<uint32_t, std::vector<CraftGameInstance::CraftRecipe>>
      cache;
  if (!cache.contains(skillID)) {
    std::vector<CraftGameInstance::CraftRecipe> r;
    for (auto k : recipes_) {
      if (k.conditions.skillID == skillID) {
        r.emplace_back(k);
      }
    }
    cache[skillID] = r;
  }
  return cache.at(skillID);
}

}  // namespace sdlms
