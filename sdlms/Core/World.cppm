module;

#include "entt/entt.hpp"

export module core:world;

void Transform_construct(entt::registry& registry, entt::entity entity);

export struct World
{
   static inline entt::registry registry;

   static void init();
};

