#include "mappers/mapper1.h"
#include <stdexcept>

Mapper1::Mapper1( u16 prg_size, u16 chr_size ) : Mapper( prg_size, chr_size ) { UpdateOffsets(); }

void Mapper1::UpdateOffsets()
{
    // Extract PRG and CHR bank modes from the control register
    u8 prg_mode = ( _control >> 2 ) & 0x03;
    u8 chr_mode = ( _control >> 4 ) & 0x01;

    u32 prg_size = static_cast<u32>( GetPRGROMSize() );
    u32 chr_size = static_cast<u32>( GetCHRROMSize() );

    // Update PRG bank offsets
    switch ( prg_mode )
    {
        case 0:
        case 1:
            // 32 KB PRG ROM mode (ignore lowest bit of _prg_bank)
            _prg_bank &= 0xFE; // Clear lowest bit
            _prg_bank_offset_0 = ( _prg_bank * 0x4000 ) % prg_size;
            _prg_bank_offset_1 = ( _prg_bank_offset_0 + 0x4000 ) % prg_size;
            break;
        case 2:
            // Fix first bank at $8000, switch 16 KB bank at $C000
            _prg_bank_offset_0 = 0;
            _prg_bank_offset_1 = ( _prg_bank * 0x4000 ) % prg_size;
            break; // Added missing break
        case 3:
            // Switch 16 KB bank at $8000, fix last bank at $C000
            _prg_bank_offset_0 = ( _prg_bank * 0x4000 ) % prg_size;
            _prg_bank_offset_1 = ( prg_size - 0x4000 ) % prg_size;
            break;
        default:
            throw std::runtime_error( "Invalid PRG bank mode" );
    }

    // Update CHR bank offsets
    if ( chr_mode == 0 )
    {
        // 8 KB CHR ROM mode (ignore lowest bit of _chr_bank_0)
        _chr_bank_0 &= 0xFE; // Clear lowest bit
        _chr_bank_offset_0 = ( _chr_bank_0 * 0x1000 ) % chr_size;
        _chr_bank_offset_1 = _chr_bank_offset_0 + 0x1000 * 4; // For completeness
    }
    else
    {
        // 4 KB CHR ROM mode
        _chr_bank_offset_0 = ( _chr_bank_0 * 0x1000 ) % chr_size;
        _chr_bank_offset_1 = ( _chr_bank_1 * 0x1000 ) % chr_size;
    }
}

u16 Mapper1::TranslateCPUAddress( u16 address )
{
    if ( address >= 0x8000 && address <= 0xFFFF )
    {
        if ( address < 0xC000 )
        {
            // Map to PRG bank 0
            u32 offset = _prg_bank_offset_0 + ( address - 0x8000 );
            return static_cast<u16>( offset % GetPRGROMSize() );
        }

        // Map to PRG bank 1
        u32 offset = _prg_bank_offset_1 + ( address - 0xC000 );
        return static_cast<u16>( offset % GetPRGROMSize() );
    }
    throw std::runtime_error( "Address out of range in TranslateCPUAddress" );
}

u16 Mapper1::TranslatePPUAddress( u16 address )
{
    if ( address >= 0x0000 && address <= 0x0FFF )
    {
        // Map to CHR bank 0
        u32 offset = _chr_bank_offset_0 + address;
        return static_cast<u16>( offset % GetCHRROMSize() );
    }
    if ( address >= 0x1000 && address <= 0x1FFF )
    {
        // Map to CHR bank 1
        u32 offset = _chr_bank_offset_1 + ( address - 0x1000 );
        return static_cast<u16>( offset % GetCHRROMSize() );
    }
    throw std::runtime_error( "Address out of range in TranslatePPUAddress" );
}

void Mapper1::HandleCPUWrite( u16 address, u8 data )
{
    if ( address < 0x8000 || address > 0xFFFF )
    {
        // Ignore writes outside of $8000 - $FFFF
        return;
    }

    // If bit 7 is set, reset shift register and set control register bit 2 to 1
    if ( ( data & 0x80 ) != 0 )
    {
        _shift_register = 0;
        _bits_loaded = 0;
        _control |= 0x0C; // Set PRG bank mode to 3
        UpdateOffsets();
        return;
    }

    // Shift bit 0 into shift register (from LSB to MSB)
    _shift_register >>= 1;
    _shift_register |= ( data & 0x01 ) << 4;
    _bits_loaded++;

    if ( _bits_loaded == 5 )
    {
        // Determine target based on address
        u16 register_select = ( address >> 13 ) & 0x03; // Bits 13-14
        u8  value = _shift_register & 0x1F;             // Bits 0-4

        switch ( register_select )
        {
            case 0:
                _control = value;
                break;
            case 1:
                _chr_bank_0 = value;
                break;
            case 2:
                _chr_bank_1 = value;
                break;
            case 3:
                _prg_bank = value;
                break;
            default:
                throw std::runtime_error( "Invalid register select value" );
        }

        // Reset shift register and bit counter, and update bank offsets
        _shift_register = 0;
        _bits_loaded = 0;
        UpdateOffsets();
    }
}

u8 Mapper1::GetMirrorMode()
{
    return _control & 0x03; // Bits 0-1
}
