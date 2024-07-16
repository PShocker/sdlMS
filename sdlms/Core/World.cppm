module;

#include "entt/entt.hpp"

export module core:world;

import components;

export struct World
{
   static inline entt::registry registry;

   static inline void sort()
   {
      registry.sort<Transform>([](const Transform &m, const Transform &n)
                               { return m.z < n.z; });
   }
};
