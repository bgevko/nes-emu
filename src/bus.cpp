
// bus.cpp

#include "apu.h"
#include "bus.h"
#include "cartridge.h"
#include "ppu.h"

// Initialize RAM with zeros and cartridge with nullptr
Bus::Bus( PPU &ppu, APU &apu ) : _ram{}, _cartridge{ nullptr }, _ppu{ ppu }, _apu{ apu } {}

u8 Bus::Read( u16 address ) const
{
    // System RAM: 0x0000 - 0x1FFF (mirrored every 2KB)
    if ( address >= 0x0000 && address <= 0x1FFF )
    {
        u16 ramAddress = address & 0x07FF; // Mirror the address
        return _ram[ramAddress];
    }

    // PPU Registers: 0x2000 - 0x3FFF (mirrored every 8 bytes)
    if ( address >= 0x2000 && address <= 0x3FFF )
    {
        u16 ppuRegister = 0x2000 + ( address & 0x0007 );
        return _ppu.ReadRegister( ppuRegister );
    }

    // APU and I/O Registers: 0x4000 - 0x401F
    if ( address >= 0x4000 && address <= 0x401F )
    {
        u16 apuAddress = address & 0x001F;

        // Handle reads from controller ports and other I/O
        return _apu.Read( apuAddress );
    }

    // Unused APU and I/O addresses: 0x4018 - 0x401F
    if ( address >= 0x4018 && address <= 0x401F )
    {
        // Return open bus value or handle as per emulator design
        return 0x00;
    }

    // Expansion ROM: 0x4020 - 0x5FFF (if applicable)
    if ( address >= 0x4020 && address <= 0x5FFF )
    {
        u16 expAddress = address - 0x4020;
        if ( _cartridge )
        {
            return _cartridge->ReadExpROM( expAddress );
        }

        // Return open bus value if no cartridge
        return 0xFF;
    }

    // SRAM (Save RAM): 0x6000 - 0x7FFF
    if ( address >= 0x6000 && address <= 0x7FFF )
    {
        u16 sramAddress = address - 0x6000;
        if ( _cartridge )
        {
            return _cartridge->ReadSRAM( sramAddress );
        }

        // Return open bus value if no cartridge
        return 0xFF;
    }

    // PRG ROM: 0x8000 - 0xFFFF
    if ( address >= 0x8000 && address <= 0xFFFF )
    {
        u16 prgAddress = address - 0x8000;
        if ( _cartridge )
        {
            return _cartridge->ReadPRG( prgAddress );
        }

        // Return open bus value if no cartridge
        return 0xFF;
    }

    // Unhandled address ranges return open bus value
    return 0xFF;
}

void Bus::Write( u16 address, u8 data )
{
    // System RAM: 0x0000 - 0x1FFF (mirrored every 2KB)
    if ( address >= 0x0000 && address <= 0x1FFF )
    {
        u16 ramAddress = address & 0x07FF; // Mirror the address
        _ram[ramAddress] = data;
        return;
    }

    // PPU Registers: 0x2000 - 0x3FFF (mirrored every 8 bytes)
    if ( address >= 0x2000 && address <= 0x3FFF )
    {
        u16 ppuRegister = 0x2000 + ( address & 0x0007 );
        _ppu.WriteRegister( ppuRegister, data );
        return;
    }

    // APU and I/O Registers: 0x4000 - 0x401F
    if ( address >= 0x4000 && address <= 0x401F )
    {
        u16 apuAddress = address & 0x001F;

        // Handle OAM DMA transfer
        if ( address == 0x4014 )
        {
            // Initiate DMA transfer to PPU OAM
            u16                 cpuBaseAddress = data << 8;
            std::array<u8, 256> dmaData{};

            // Read 256 bytes from CPU memory
            for ( u16 i = 0; i < 256; ++i )
            {
                dmaData[i] = Read( cpuBaseAddress + i );
            }

            // Write the data to PPU OAM
            _ppu.DoDMATransfer( dmaData );
            return;
        }

        _apu.Write( apuAddress, data );
        return;
    }

    // Unused APU and I/O addresses: 0x4018 - 0x401F
    if ( address >= 0x4018 && address <= 0x401F )
    {
        // Typically ignored or logged
        return;
    }

    // Expansion ROM: 0x4020 - 0x5FFF (if applicable)
    if ( address >= 0x4020 && address <= 0x5FFF )
    {
        u16 expAddress = address - 0x4020;
        if ( _cartridge )
        {
            _cartridge->WriteExpROM( expAddress, data );
        }
        return;
    }

    // SRAM (Save RAM): 0x6000 - 0x7FFF
    if ( address >= 0x6000 && address <= 0x7FFF )
    {
        u16 sramAddress = address - 0x6000;
        if ( _cartridge )
        {
            _cartridge->WriteSRAM( sramAddress, data );
        }
        return;
    }

    // PRG ROM: 0x8000 - 0xFFFF (typically read-only)
    if ( address >= 0x8000 && address <= 0xFFFF )
    {
        if ( _cartridge )
        {
            _cartridge->WritePRG( address - 0x8000, data ); // For mapper-controlled writes
        }
        return;
    }

    // Unhandled address ranges
    // Optionally log a warning or ignore
}
