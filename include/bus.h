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
    /* Bus( PPU &ppu, APU &apu ); */
    Bus( bool use_flat_memory = false );

    // Memory read/write interface
    [[nodiscard]] u8 Read( uint16_t address ) const;
    void             Write( u16 address, u8 data );

    // Load or change the cartridge during runtime
    void LoadCartridge( std::shared_ptr<Cartridge> cartridge );

  private:
    /* std::shared_ptr<Cartridge> _cartridge; // Shared ownership for dynamic life cycle management
     */

    /* PPU &_ppu; // NOLINT */
    /* APU &_apu; // NOLINT */

    bool _use_flat_memory; // For testing purposes

    std::array<u8, 65536> _flat_memory{}; // 64KB memory, for early testing

    std::array<u8, 0x0800> _ram{}; // 2KB internal cpu RAM

    // temp memory placeholders
    std::array<u8, 0x2000> _ppu_memory{};           // 8KB PPU memory (temp)
    std::array<u8, 0x0020> _apu_io_memory{};        // 32 bytes APU and I/O registers
    std::array<u8, 0x1FE0> _expansion_rom_memory{}; // 8160 bytes expansion ROM memory
    std::array<u8, 0x2000> _sram_memory{};          // 8KB save RAM memory
    std::array<u8, 0x8000> _prg_rom_memory{};       // 32KB PRG ROM memory
};
