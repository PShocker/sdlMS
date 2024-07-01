#include "Timer.h"

Timer::Timer(const std::function<Uint32(Uint32, void *)> &callback, int time) : callback(callback), time(time)
{
}
