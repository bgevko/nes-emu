#include "apu.h"

APU::APU() : _registers{} {}

u8 APU::Read( u16 address ) const
{
    // APU register range is 0x4000 - 0x401F, so map to 0x00 - 0x1F
    u16 apuAddress = address & 0x1F; // Limit to 0x00 - 0x1F
    return _registers[apuAddress];
}

void APU::Write( u16 address, u8 data ) // NOLINT
{
    // APU register range is 0x4000 - 0x401F, so map to 0x00 - 0x1F
    u16 apuAddress = address & 0x1F; // Limit to 0x00 - 0x1F
    _registers[apuAddress] = data;
}
