#include "cartridge.h"
#include "cpu.h"
#include <array>
#include <cstddef>
#include <fstream>
#include <ios>
#include <memory>
#include <stdexcept>
#include <string>

// Include all available mappers
#include "mappers/mapper0.h"
#include "mappers/mapper1.h"

Cartridge::Cartridge( const std::string &file_path )
{
    std::ifstream rom_file( file_path, std::ios::binary );
    if ( !rom_file.is_open() )
    {
        throw std::runtime_error( "Failed to open ROM file: " + file_path );
    }

    // Can adjust later, but we'll say files bigger than 5 MiB shouldn't be valid
    constexpr size_t max_rom_size = static_cast<const size_t>( 5 * 1024 * 1024 );
    rom_file.seekg( 0, std::ios::end );
    size_t const file_size = static_cast<size_t>( rom_file.tellg() );
    if ( file_size > max_rom_size )
    {
        throw std::runtime_error( "ROM file too large" );
    }

    // Seek back to the beginning
    rom_file.seekg( 0, std::ios::beg );

    // Read the header
    std::array<char, 16> header{};
    if ( !rom_file.read( header.data(), header.size() ) )
    {
        if ( rom_file.eof() )
        {
            throw std::runtime_error( "Failed to read ROM header: Unexpected end of file." );
        }
        if ( rom_file.fail() )
        {
            throw std::runtime_error( "Failed to read ROM header: I/O error." );
        }
        if ( rom_file.bad() )
        {
            throw std::runtime_error( "Failed to read ROM header: Fatal I/O error." );
        }
        if ( rom_file.good() )
        {
            throw std::runtime_error( "Failed to read ROM header: No error." );
        }
    }

    // First four bytes, should be "NES\x1A", if not, let's bail
    if ( header[0] != 'N' || header[1] != 'E' || header[2] != 'S' || header[3] != 0x1A )
    {
        throw std::runtime_error( "Invalid ROM file" );
    }

    // Extract header info
    _mirror_mode = header[6] & 0b00000001;
    size_t const prg_size_bytes = static_cast<size_t>( header[4] * 16 * 1024 );
    size_t const chr_size_bytes = static_cast<size_t>( header[5] * 8 * 1024 );

    // Set up the mapper
    u8 const flags6 = header[6];
    u8 const flags7 = header[7];
    u8 const mapper_id = ( flags7 & 0b11110000 ) | ( flags6 >> 4 );
    switch ( mapper_id )
    {
        case 0:
            _mapper = std::make_shared<Mapper0>( prg_size_bytes, chr_size_bytes );
            break;
        case 1:
            _mapper = std::make_shared<Mapper1>( prg_size_bytes, chr_size_bytes );
            break;
        default:
            throw std::runtime_error( "Unsupported mapper: " + std::to_string( mapper_id ) );
    };

    // Cart flags
    _has_battery = ( flags6 & 0b00000010 );
    _four_screen_mode = ( flags6 & 0b00001000 );

    // Skip trainer if present
    bool const has_trainer = ( flags6 & 0b00000100 ) != 0;
    if ( has_trainer )
    {
        rom_file.seekg( 512, std::ios::cur );
    }

    // Read PRG and CHR ROM data
    _prg_rom.resize( prg_size_bytes );
    rom_file.read( reinterpret_cast<char *>( _prg_rom.data() ), // NOLINT
                   static_cast<std::streamsize>( prg_size_bytes ) );

    // Some games use CHR RAM when CHR ROM is not present
    if ( chr_size_bytes == 0 )
    {
        _chr_ram.resize( 8192 );
    }
    else
    {
        _chr_rom.resize( chr_size_bytes );
        rom_file.read( reinterpret_cast<char *>( _chr_rom.data() ), // NOLINT
                       static_cast<std::streamsize>( chr_size_bytes ) );
    }

    // Close the file
    rom_file.close();
}

[[nodiscard]] u8 Cartridge::ReadPRG( u16 address )
{
    return _prg_rom[_mapper->TranslateCPUAddress( address )];
}

[[nodiscard]] u8 Cartridge::ReadCHR( u16 address ) const
{
    u16 const translated_address = _mapper->TranslatePPUAddress( address );
    if ( !_chr_rom.empty() )
    {
        // Read from CHR ROM
        return _chr_rom[translated_address];
    }
    if ( !_chr_ram.empty() )
    {
        // Read from CHR RAM
        return _chr_ram[translated_address];
    }

    // No CHR ROM or RAM
    return 0xFF;
}

void Cartridge::WriteCHR( u16 address, u8 data )
{
    u16 const translated_address = _mapper->TranslatePPUAddress( address );
    if ( !_chr_ram.empty() )
    {
        // Write to CHR RAM
        _chr_ram[translated_address] = data;
    }
    else
    {
        // CHR ROM is read-only; writes are ignored or could trigger an error
        // For now, we can ignore writes to CHR ROM
    }
}

u8 Cartridge::GetMirrorMode()
{
    u8 mode = _mapper->GetMirrorMode();
    _mirror_mode = mode;
    return _mirror_mode;
}

void Cartridge::WritePRG( u16 address, u8 data ) { _mapper->HandleCPUWrite( address, data ); }
