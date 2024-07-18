module;
#include "wz/Property.hpp"
#include <optional>

export module components:foothold;

export struct FootHold
{
    int page;
    int zmass;
    int prev;
    int next;

    int x1;
    int y1;
    int x2;
    int y2;

    int l;
    int r;
    int t;
    int b;

    std::optional<float> k = std::nullopt; // 斜率,若k值不存在,可以判断是否是墙面
    std::optional<float> intercept = std::nullopt;

    static inline std::unordered_map<int, FootHold *> fhs;

    std::optional<float> get_x(float y);
    std::optional<float> get_y(float x);

    FootHold() = default;
};

