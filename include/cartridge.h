#pragma once

#include "cpu.h"
#include <string>
#include <vector>

class Cartridge
{
  public:
    Cartridge( const std::string &file_path );
    [[nodiscard]] u8 ReadPRG( u16 address );
    [[nodiscard]] u8 ReadCHR( u16 address ) const;

  private:
    // PRG and CHR ROM data
    std::vector<u8> _prg_rom;
    std::vector<u8> _chr_rom;

    // Header information
    u8   _prg_rom_size = 0; // in 16 KB units
    u8   _chr_rom_size = 0; // in 8 KB units
    u8   _mapper = 0;
    u8   _mirroring = 0; // 0: horizontal, 1: vertical
    bool _has_battery = false;
    bool _has_trainer = false;
    bool _four_screen_mode = false;
};
