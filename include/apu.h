#pragma once
#include <array>
#include <cstdint>

using u8 = uint8_t;
using u16 = uint16_t;

class APU
{
  public:
    APU();

    [[nodiscard]] u8 Read( u16 address ) const;
    void             Write( u16 address, u8 data );

  private:
    std::array<u8, 0x20> _registers; // 32 bytes for APU and I/O registers (0x4000 - 0x401F)
};
