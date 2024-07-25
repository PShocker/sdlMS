module;

#include <optional>

export module components:border;

export struct Border
{
    std::optional<int> l = std::nullopt;
    std::optional<int> r = std::nullopt;
    std::optional<int> t = std::nullopt;
    std::optional<int> b = std::nullopt;
};