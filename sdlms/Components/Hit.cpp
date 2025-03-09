#include "Hit.h"

int HitWarp::real_damage()
{
    // 生成0到1之间的随机浮动因子
    double random_factor = (std::rand() % 101) / 100.0;
    random_factor = range + (random_factor * 0.1);
    return damage * random_factor;
}
