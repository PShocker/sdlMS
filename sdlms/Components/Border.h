#pragma once

#include <optional>

struct Border
{
    std::optional<int> l = std::nullopt;
    std::optional<int> r = std::nullopt;
    std::optional<int> t = std::nullopt;
    std::optional<int> b = std::nullopt;
};