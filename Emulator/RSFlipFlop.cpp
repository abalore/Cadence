#include "RSFlipFlop.h"

RSFlipFlop::RSFlipFlop()
{
    Q = false;
}

void RSFlipFlop::Set()
{
    Q = true;
}

void RSFlipFlop::Reset()
{
    Q = false;
}
