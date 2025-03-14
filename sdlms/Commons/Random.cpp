#include "Random.h"

float generate_random(float m, float n)
{
    auto a = std::min(m, n);
    auto b = std::max(m, n); // 范围上限

    // 生成 [a, b) 之间的随机浮点数
    auto random_value = a + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (b - a);

    return random_value;
}