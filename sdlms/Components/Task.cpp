#include "Task.h"

Task::Task(std::function<void(Entity *ent, World &world)> func, int delay) : func(func), delay(delay)
{
}