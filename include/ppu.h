#pragma once
#include <array>
#include <cstdint>

using u8 = uint8_t;
using u16 = uint16_t;

class Cartridge;

class PPU
{
  public:
    PPU( Cartridge *cartridge );

    // Read and write access to PPU registers (placeholder functionality)
    [[nodiscard]] u8 ReadRegister( u16 address ) const;
    void             WriteRegister( u16 address, u8 data );

    // DMA Transfer (OAM DMA)
    void DoDMATransfer( const std::array<u8, 256> &data );

    // Memory access (used internally)
    [[nodiscard]] u8 Read( u16 address ) const;
    void             Write( u16 address, u8 data );

  private:
    Cartridge *_cartridge;

    std::array<u8, 0x4000> _vram;    // VRAM (2KB for nametables)
    std::array<u8, 32>     _palette; // Palette RAM indeces (32 bytes)
    std::array<u8, 256>    _oam;     // 256 bytes of OAM memory for sprites

    enum class Mirroring
    {
        Horizontal,
        Vertical
    };
    Mirroring _mirroring;
};
