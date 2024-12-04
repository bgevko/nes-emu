#include "cartridge.h"
#include "cpu.h"
#include <array>
#include <cstddef>
#include <fstream>
#include <ios>
#include <limits>
#include <stdexcept>
#include <string>

Cartridge::Cartridge( const std::string &file_path )
{
    std::ifstream rom_file( file_path, std::ios::binary );
    if ( !rom_file.is_open() )
    {
        throw std::runtime_error( "Failed to open ROM file: " + file_path );
    }

    // Read the header
    std::array<char, 16> char_header{};
    if ( !rom_file.read( char_header.data(), char_header.size() ) )
    {
        throw std::runtime_error( "Failed to read ROM header" );
    }

    // First four bytes, should be "NES\x1A", if not, let's bail
    if ( char_header[0] != 'N' || char_header[1] != 'E' || char_header[2] != 'S' ||
         char_header[3] != 0x1A )
    {
        throw std::runtime_error( "Invalid ROM file" );
    }

    // Extract header info
    _prg_rom_size = char_header[4];
    _chr_rom_size = char_header[5];

    // Extract header info
    u8 const flags6 = char_header[6];
    u8 const flags7 = char_header[7];
    _mapper = ( flags7 & 0b11110000 ) | ( flags6 >> 4 );
    _mirroring = flags6 & 0b00000001;
    _has_battery = ( flags6 & 0b00000010 ) != 0;
    _has_trainer = ( flags6 & 0b00000100 ) != 0;
    _four_screen_mode = ( flags6 & 0b00001000 ) != 0;

    // For simplicity, only support mapper 0 (NROM) initially
    if ( _mapper != 0 )
    {
        throw std::runtime_error( "Unsupported mapper number: " + std::to_string( _mapper ) );
    }

    // Skip trainer if present
    if ( _has_trainer )
    {
        rom_file.seekg( 512, std::ios::cur );
    }

    // Read program ROM data (PRG)
    size_t const prg_rom_bytes = static_cast<size_t>( _prg_rom_size * 8 * 1024 );

    // Safety check to make sure we can convert size_t to std::streamsize, which is an usigned to a
    // signed conversion
    if ( prg_rom_bytes > static_cast<size_t>( std::numeric_limits<std::streamsize>::max() ) )
    {
        throw std::runtime_error( "PRG ROM size exceeds maximum allowable size." );
    }
    _prg_rom.resize( prg_rom_bytes );

    // Disable lint here because we need to read raw bytes into _prg_rom, which is a vector of u8
    rom_file.read( reinterpret_cast<char *>( _prg_rom.data() ), // NOLINT
                   static_cast<std::streamsize>( prg_rom_bytes ) );

    // Read character ROM data (CHR)
    size_t const chr_rom_bytes = static_cast<size_t>( _chr_rom_size * 8 * 1024 );
    if ( chr_rom_bytes > static_cast<size_t>( std::numeric_limits<std::streamsize>::max() ) )
    {
        throw std::runtime_error( "CHR ROM size exceeds maximum allowable size." );
    }
    _chr_rom.resize( chr_rom_bytes );
    rom_file.read( reinterpret_cast<char *>( _chr_rom.data() ), // NOLINT
                   static_cast<std::streamsize>( chr_rom_bytes ) );

    // Close the file
    rom_file.close();
}

[[nodiscard]] u8 Cartridge::ReadPRG( u16 address )
{
    if ( _prg_rom_size == 1 )
    {
        // 16 KB PRG ROM, mirror the bank
        address = address & 0x3FFF; // Mask to 16 KB
    }
    else if ( _prg_rom_size == 2 )
    {
        // 32 KB PRG ROM, no mirroring needed
        address = address & 0x7FFF; // Mask to 32 KB
    }
    else
    {
        // Unsupported PRG ROM size
        throw std::runtime_error( "Unsupported PRG ROM size: " + std::to_string( _prg_rom_size ) );
    }

    return _prg_rom[address];
}

u8 Cartridge::ReadCHR( u16 address ) const { return _chr_rom[address]; }
