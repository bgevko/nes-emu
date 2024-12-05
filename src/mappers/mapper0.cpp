/*
  Mapper 0 is the simplest mapper. It's just a ROM chip with no bank switching.
*/
#include "mappers/mapper0.h"
#include "mappers/mapper.h"
#include <cstddef>
#include <stdexcept>

[[nodiscard]] u16 Mapper0::TranslateCPUAddress( u16 address )
{
    if ( address >= 0x8000 && address <= 0xFFFF )
    {
        // 16 KiB PRG ROM
        if ( GetPRGROMSize() == static_cast<size_t>( 16 * 1024 ) )
        {
            // Mirror 16 KiB PRG ROM across 0x8000 - 0xFFFF
            return ( address - 0x8000 ) % ( 16 * 1024 );
        }

        // 32 KiB PRG ROM
        if ( GetPRGROMSize() == static_cast<size_t>( 32 * 1024 ) )
        {
            // Use 32 KiB PRG ROM directly
            return address - 0x8000;
        }

        // Unsupported PRG ROM size
        throw std::runtime_error( "Unsupported PRG ROM size for Mapper 0" );
    }
    throw std::runtime_error( "Address out of range in TranslateCPUAddress" );
}

[[nodiscard]] auto Mapper0::TranslatePPUAddress( u16 address ) -> u16
{
    if ( address >= 0x0000 && address <= 0x1FFF )
    {
        return address;
    }
    return 0xFF;
}

void Mapper0::HandleCPUWrite( u16 address, u8 data )
{
    // No writable registers
    (void) address;
    (void) data;
}
