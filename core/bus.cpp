#include "bus.h"
#include "cartridge.h"
#include "ppu.h"
#include <iostream>

// Constructor to initialize the bus with a flat memory model
Bus::Bus() : cpu( this ), ppu( this ), cartridge( this )
{
}

/*
################################
||          CPU Read          ||
################################
*/
u8 Bus::Read( const u16 address, bool debugMode )
{
    if ( _useFlatMemory ) {
        return _flatMemory.at( address );
    }

    // System RAM: 0x0000 - 0x1FFF (mirrored every 2KB)
    if ( address >= 0x0000 && address <= 0x1FFF ) {
        return _ram.at( address & 0x07FF );
    }

    // PPU Registers: 0x2000 - 0x3FFF (mirrored every 8 bytes)
    if ( address >= 0x2000 && address <= 0x3FFF ) {
        // ppu read will go here. For now, return from temp private member of bus
        const u16 ppuRegister = 0x2000 + ( address & 0x0007 );
        return ppu.HandleCpuRead( ppuRegister, debugMode );
    }

    // APU and I/O Registers: 0x4000 - 0x401F
    if ( address >= 0x4000 && address <= 0x401F ) {
        // Handle reads from controller ports and other I/O
        // apu read will go here. For now, return from temp private member of bus
        return _apuIoMemory.at( address & 0x001F );
    }

    // 4020 and up is cartridge territory
    if ( address >= 0x4020 && address <= 0xFFFF ) {
        return cartridge.Read( address );
    }

    // Unhandled address ranges return open bus value
    std::cout << "Unhandled read from address: " << std::hex << address << "\n";
    return 0xFF;
}

/*
################################
||          CPU Write         ||
################################
*/
void Bus::Write( const u16 address, const u8 data )
{

    if ( _useFlatMemory ) {
        _flatMemory.at( address ) = data;
        return;
    }

    // System RAM: 0x0000 - 0x1FFF (mirrored every 2KB)
    if ( address >= 0x0000 && address <= 0x1FFF ) {
        _ram.at( address & 0x07FF ) = data;
        return;
    }

    // PPU Registers: 0x2000 - 0x3FFF (mirrored every 8 bytes)
    if ( address >= 0x2000 && address <= 0x3FFF ) {
        const u16 ppuRegister = 0x2000 + ( address & 0x0007 );
        ppu.HandleCpuWrite( ppuRegister, data );
        return;
    }

    // PPU DMA: 0x4014
    if ( address == 0x4014 ) {
        // DEBUG: Disable
        //  dmaInProgress = true;
        //  dmaAddr = data << 8;
        return;
    }

    // APU and I/O Registers: 0x4000 - 0x401F
    if ( address >= 0x4000 && address <= 0x401F ) {
        _apuIoMemory.at( address & 0x001F ) = data; // temp
        return;
    }

    // 4020 and up is cartridge territory
    if ( address >= 0x4020 && address <= 0xFFFF ) {
        cartridge.Write( address, data );
        return;
    }
    // Unhandled address ranges
    // Optionally log a warning or ignore
    std::cout << "Unhandled write to address: " << std::hex << address << "\n";
}

void Bus::ProcessDma()
{
    const u64 cycle = cpu.GetCycles();

    // Wait first read is on an odd cycle, wait it out.
    if ( dmaOffset == 0 && cycle % 2 == 1 ) {
        cpu.Tick();
        return;
    }

    // Read into OAM on even, load next byte on odd
    if ( cycle % 2 == 0 ) {
        auto data = Read( dmaAddr | dmaOffset );
        cpu.Tick();
        ppu.oam.at( dmaOffset++ ) = data;
    } else {
        dmaInProgress = dmaOffset < 256;
        cpu.Tick();
    }
}

bool Bus::Clock()
{
    if ( dmaInProgress ) {
        ProcessDma();
    } else {
        cpu.DecodeExecute();
    }

    if ( ppu.nmiReady ) {
        ppu.nmiReady = false;
        cpu.NMI();
    }

    if ( ppu.frameComplete ) {
        ppu.frameComplete = false;
        return true;
    }

    return false;
}

/*
################################
||        Debug Methods       ||
################################
*/
[[nodiscard]] bool Bus::IsTestMode() const
{
    return _useFlatMemory;
}
void Bus::DebugReset()
{
    cpu.SetCycles( 0 );
    cpu.Reset();
    ppu.Reset();
}
