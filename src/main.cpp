// cpu.cpp

#include "bus.h"
#include "cpu.h"
#include <iostream>

auto main() -> int
{
    Bus bus = Bus();
    CPU cpu = CPU( &bus );

    std::cout << "Hello emulator. All is working." << '\n';
    return 0;
}
