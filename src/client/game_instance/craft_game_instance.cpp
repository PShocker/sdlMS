#include "src/client/game_instance/craft_game_instance.h"

#include "src/common/wz/wz_resource.h"

#include <wz/Property.h>

#include <cstdint>
#include <flat_map>


namespace sdlms {

void CraftGameInstance::Load(const CharacterSaveData& cs) {
  datas_ = {};
  for (auto& c : cs.craft) {
    CraftData d;
    d.exp = c.exp;
    d.level = c.level;
    d.type = (CraftType)c.type;
    datas_[c.type] = d;
  }
}

void CraftGameInstance::InitRecipe() {
  auto node = WzResource::etc_->find(u"CraftRecipe.img");
  for (auto [k, v] : *node->get_children()) {
    for (auto [k2, v2] : *v[0]->get_children()) {
      for (auto [k3, v3] : *v2[0]->get_children()) {
        CraftRecipe cr;
        cr.coolTimeSec =
            static_cast<wz::Property<int>*>(v3[0]->get_child(u"CoolTimeSec"))
                ->get();
        cr.processTimeMS =
            static_cast<wz::Property<int>*>(v3[0]->get_child(u"ProcessTimeMS"))
                ->get();
        cr.meso =
            static_cast<wz::Property<int>*>(v3[0]->get_child(u"Meso"))->get();
        cr.conditions.reqLevel =
            static_cast<wz::Property<int>*>(
                v3[0]->find(u"Conditions/SkillCheck/reqSkillLevel"))
                ->get();
        cr.conditions.skillID =
            static_cast<wz::Property<int>*>(
                v3[0]->find(u"Conditions/SkillCheck/SkillID"))
                ->get();
        for (auto [k4, v4] :
             *v3[0]->get_child(u"Ingredients")->get_children()) {
          CraftInredient cid;
          cid.count =
              static_cast<wz::Property<int>*>(v3[0]->get_child(u"count"))
                  ->get();
          cid.item =
              static_cast<wz::Property<int>*>(v3[0]->get_child(u"item"))->get();
          cr.ingredients.emplace_back(cid);
        }
        cr.result.item =
            static_cast<wz::Property<int>*>(v3[0]->find(u"Results/item"))
                ->get();
        cr.result.count =
            static_cast<wz::Property<int>*>(v3[0]->find(u"Results/count"))
                ->get();
        cr.result.exp =
            static_cast<wz::Property<int>*>(v3[0]->find(u"Results/exp"))->get();
        recipes_.emplace_back(cr);
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
