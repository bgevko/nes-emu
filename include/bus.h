#pragma once
#include "cpu.h"
#include "ppu.h"
#include <array>
#include <cstdint>
#include <memory>

using u8 = uint8_t;
using u16 = uint16_t;
using u64 = uint64_t;
using s16 = int16_t;

// forward declarations
class Cartridge;
class CPU;
class PPU;
class APU;

class Bus
{
  public:
    // Will eventually pass other components to the constructor
    // Bus( PPU *ppu, APU *apu, bool use_flat_memory = false );

    // Initialized with flat memory disabled by default. Enabled in json tests only
    // explicit Bus( PPU *ppu, bool use_flat_memory = false );
    Bus( bool _use_flat_memory = false );

    CPU                        cpu;       // NOLINT
    PPU                        ppu;       // NOLINT
    std::shared_ptr<Cartridge> cartridge; // NOLINT

    // Memory read/write interface
    [[nodiscard]] u8 Read( uint16_t address );
    void             Write( u16 address, u8 data );

    // Load or change the cartridge during runtime
    void LoadCartridge( std::shared_ptr<Cartridge> cartridge );

    // Is test mode
    [[nodiscard]] bool IsTestMode() const;

  private:
    // Flat memory for early implementation
    bool                  _use_flat_memory; // For testing purposes
    std::array<u8, 65536> _flat_memory{};   // 64KB memory, for early testing

    // CPU RAM, this can stay in this file
    std::array<u8, 0x0800> _ram{}; // 2KB internal cpu RAM

    // Stubs
    std::array<u8, 0x0020> _apu_io_memory{}; // 32 bytes APU and I/O registers
};
