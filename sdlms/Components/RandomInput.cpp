#include "RandomInput.h"

RandomInput::RandomInput(int val)
{
    count = std::rand() % val + 100;
}