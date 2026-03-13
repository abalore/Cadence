#include "Counter.h"

Counter::Counter(BYTE mask)
{
    this->mask = mask;
    value = 0;
}

void Counter::Step()
{
    value++;
    value &= mask;
}

void Counter::Reset()
{
    value = 0;
}
