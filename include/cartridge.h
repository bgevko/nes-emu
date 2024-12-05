#pragma once

#include "cpu.h"
#include "mappers/mapper.h"
#include <string>
#include <vector>
#include <memory>

class Cartridge
{
  public:
    Cartridge( const std::string &file_path );
    [[nodiscard]] u8 ReadPRG( u16 address );
    [[nodiscard]] u8 ReadCHR( u16 address ) const;
    void             WriteCHR( u16 address, u8 data );
    void             WritePRG( u16 address, u8 data );
    [[nodiscard]] u8 GetMirrorMode();

  private:
    // PRG and CHR ROM data
    std::vector<u8> _prg_rom;
    std::vector<u8> _chr_rom;
    std::vector<u8> _chr_ram;

    // Mapper
    std::shared_ptr<Mapper> _mapper;

    // Cartridge flags
    u8 _has_battery = 0;
    u8 _four_screen_mode = 0;
    u8 _mirror_mode = 0;

    enum MirrorMode : u8
    {
        Horizontal,
        Vertical,
        FourScreen
    };
};
