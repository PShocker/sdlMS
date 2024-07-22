module;

#include "entt/entt.hpp"

export module core:world;

export struct World
{
   static inline entt::registry registry;

   static inline bool zsort = false;
};
