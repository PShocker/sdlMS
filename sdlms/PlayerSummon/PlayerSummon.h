#pragma once

#include <string>
#include <functional>
#include "Components/Components.h"
#include "entt/entt.hpp"

int summon_4111002(entt::entity ent);

struct PlayerSummon
{
    static const inline std::unordered_map<int, std::function<int(entt::entity)>> Summons = {
        {4111002, summon_4111002},

    };
};
