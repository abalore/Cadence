#include "DFlipFlop.h"

DFlipFlop::DFlipFlop()
{
    D = 0;
    Q = 0;
}

void DFlipFlop::Clock()
{
    Q = D;
}
