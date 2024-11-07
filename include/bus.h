// bus.h
#pragma once

#include "cpu.h"

// forward declarations
class Cartridge;
class PPU;
class APU;

class Bus
{
  public:
    Bus( PPU &ppu, APU &apu );

    // Memory read/write interface
    [[nodiscard]] u8 Read( uint16_t address ) const;
    void             Write( u16 address, u8 data );

    // Load or change the cartridge during runtime
    void LoadCartridge( std::shared_ptr<Cartridge> cartridge );

  private:
    std::shared_ptr<Cartridge> _cartridge; // Shared ownership for dynamic life cycle management

    PPU &_ppu; // NOLINT
    APU &_apu; // NOLINT

    std::array<u8, 0x0800> _ram; // 2KB internal cpu RAM
};
