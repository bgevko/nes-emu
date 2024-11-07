#include "cartridge.h"

Cartridge::Cartridge( const std::vector<u8> &prgRomData )
    : _prgRom( prgRomData ), _sram( 0x2000, 0 ) // Initialize SRAM to 8 KB
{
}

u8 Cartridge::ReadPRG( u16 address ) const
{
    // TODO: Mirroring
    return _prgRom[address];
}

u8 Cartridge::ReadCHR( u16 address ) const { return _chrRom[address]; }

u8 Cartridge::ReadSRAM( u16 address ) const { return _sram[address]; }

u8 Cartridge::ReadExpROM( u16 address ) const { return _expRom[address]; }

void Cartridge::WriteSRAM( u16 address, u8 data ) { _sram[address] = data; }
void Cartridge::WriteExpROM( u16 address, u8 data ) { _expRom[address] = data; }
void Cartridge::WritePRG( u16 address, u8 data ) { _prgRom[address] = data; }
void Cartridge::WriteCHR( u16 address, u8 data ) { _chrRom[address] = data; }
