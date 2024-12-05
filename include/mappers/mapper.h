#pragma once
#include <cstddef>
#include <cstdint>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// Base mapper class
// Methods that are shared by all mappers should go here.
// Each mapper will inherit this and be defined in a separate file.
class Mapper
{
  public:
    Mapper( size_t prg_size_bytes, size_t chr_size_bytes )
        : _prg_KiB_capacity( prg_size_bytes ), _chr_KiB_capacity( chr_size_bytes )
    {
    }
    Mapper( const Mapper & ) = delete;             // Copy constructor
    Mapper &operator=( const Mapper & ) = default; // Copy assignment operator
    Mapper( Mapper && ) = delete;                  // Move constructor
    Mapper &operator=( Mapper && ) = default;      // Move assignment
    virtual ~Mapper() = default;                   // Destructor

    // Translate CPU bus address to PRG ROM offset
    virtual u16 TranslateCPUAddress( u16 address ) = 0;

    // Translate PPU bus address to CHR ROM offset
    virtual u16 TranslatePPUAddress( u16 address ) = 0;

    // Handle CPU writes for control purposes (bank switching, mirroring, etc.)
    virtual void HandleCPUWrite( u16 address, u8 data ) = 0;

    // Get mirror mode
    [[nodiscard]] virtual u8 GetMirrorMode() = 0;

    // Getters
    [[nodiscard]] size_t GetPRGROMSize() const { return _prg_KiB_capacity; }
    [[nodiscard]] size_t GetCHRROMSize() const { return _chr_KiB_capacity; }

  private:
    size_t _prg_KiB_capacity;
    size_t _chr_KiB_capacity;
};
