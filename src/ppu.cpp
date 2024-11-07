
#include "cartridge.h"
#include "ppu.h"
#include <algorithm>

PPU::PPU( Cartridge *cartridge )
    : _cartridge( cartridge ), _vram{}, _palette{}, _oam{}, _mirroring( Mirroring::Horizontal )
{
}

u8 PPU::Read( u16 address ) const
{
    address &= 0x3FFF; // Mirror addresses above 0x3FFF

    // Pattern tables
    if ( address >= 0x0000 && address <= 0x1FFF )
    {
        /* return _cartridge->ReadCHR( address ); */
        // TODO: Implement
        return 0;
    }
    // Nametables
    if ( address >= 0x2000 && address <= 0x3EFF )
    {
        // TODO: Implement
        return _vram[address];
    }
    // Palette RAM indexes (mirrored every 32 bytes)
    if ( address >= 0x3F00 && address <= 0x3FFF )
    {
        // TODO: Implement
        return _palette[address];
    }
    return 0x00;
}

void PPU::Write( u16 address, u8 data )
{
    address &= 0x3FFF; // Mirror addresses above 0x3FFF

    // Pattern tables
    if ( address >= 0x0000 && address <= 0x1FFF )
    {
        /* _cartridge->WriteCHR( address, data ); */
        // TODO: Implement
        return;
    }
    // Nametables
    if ( address >= 0x2000 && address <= 0x3EFF )
    {
        // Nametable memory with mirroring
        // TODO: Implement
        return;
    }
    // Palette RAM indexes (mirrored every 32 bytes)
    if ( address >= 0x3F00 && address <= 0x3FFF )
    {
        // TODO: Implement
        return;
    }
}

void PPU::DoDMATransfer( const std::array<u8, 256> &data )
{
    // Copy data into OAM memory
    std::copy( data.begin(), data.end(), _oam.begin() );
}

u8 PPU::ReadRegister( u16 address ) const
{
    // Implement reading from PPU registers (0x2000 - 0x2007)
    // This typically involves status registers and other control registers
    return _vram[address]; // stub
}

void PPU::WriteRegister( u16 address, u8 data )
{
    // Implement writing to PPU registers (0x2000 - 0x2007)
    // This includes control registers like PPUCTRL, PPUMASK, etc.
    _vram[address] = data; // stub
}
