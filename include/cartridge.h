#pragma once
#include <cstdint>
#include <vector>

using u8 = uint8_t;
using u16 = uint16_t;

class Cartridge
{
  public:
    Cartridge( const std::vector<u8> &prgRomData );

    // Read and write access to PRG ROM and SRAM (Save RAM)
    [[nodiscard]] u8 ReadPRG( u16 address ) const;
    [[nodiscard]] u8 ReadCHR( u16 address ) const;
    [[nodiscard]] u8 ReadSRAM( u16 address ) const;
    [[nodiscard]] u8 ReadExpROM( u16 address ) const;

    void WriteSRAM( u16 address, u8 data );
    void WriteExpROM( u16 address, u8 data );
    void WritePRG( u16 address, u8 data );
    void WriteCHR( u16 address, u8 data );

  private:
    std::vector<u8> _prgRom; // PRG ROM data loaded from the cartridge file
    std::vector<u8> _chrRom; // CHR ROM data loaded from the cartridge
    std::vector<u8> _sram;   // Save RAM (SRAM), typically 8 KB
    std::vector<u8> _expRom; // Expansion ROM, rarely used
};
