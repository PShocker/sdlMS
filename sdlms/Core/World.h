#pragma once

#include "entt/entt.hpp"
#include <unordered_map>
#include <string>

struct World
{
   // diff map have own registery
   // static inline std::unordered_map<uint32_t, entt::registry> registrys;
   // current registry
   static inline entt::registry *registry = new entt::registry{};

   static inline bool zindex = false;

   static inline std::vector<entt::entity> destory;

   struct TransPort
   {
      static inline uint32_t id = 0;
      static inline std::u16string tn = u"sp";
   };

   static void load_map(int id);
};
