#pragma once
#include "mappers/mapper-base.h"
#include <array>
#include <cstdint>

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using s16 = std::int16_t;

using namespace std;

// PPU Status
enum Status : u8
{
    SpriteOverflow = 1 << 5, // 0b00100000
    SpriteZeroHit = 1 << 6,  // 0b01000000
    VerticalBlank = 1 << 7   // 0b10000000
};

// Control Register Status
enum ControlFlag : u8
{
    NametableX = 1 << 0,             // Horizontal selection, 0 = left, 1 = right
    NametableY = 1 << 1,             // Vertical selection, 0 = top, 1 = bottom
    VramIncrementMode = 1 << 2,      // 0 = add 1, 1 = add 32
    SpritePatternTable = 1 << 3,     // 0 = $0000, 1 = $1000
    BackgroundPatternTable = 1 << 4, // 0 = $0000, 1 = $1000
    SpriteSize = 1 << 5,             // 0 = 8x8, 1 = 8x16
    MasterSlaveSelect = 1 << 6,      // Unused
    NmiEnable = 1 << 7               // Enables Non-Maskable Interrupts
};

class PPU
{
  public:
    PPU( bool isDisabled = false );

    // Getters
    [[nodiscard]] s16 GetScanline() const;
    [[nodiscard]] u16 GetCycles() const;
    [[nodiscard]] u8  GetControlFlag( ControlFlag flag ) const;

    // Setters
    void SetIsPpuReadingPpuStatus( bool isReading );

    // Reads / Writes
    [[nodiscard]] u8 HandleCpuRead( u16 addr );
    void             HandleCpuWrite( u16 addr, u8 data );

    // PPU Methods
    [[nodiscard]] u8         ReadPatternTable( u16 addr );
    [[nodiscard]] u8         ReadNameTable( u16 addr );
    [[nodiscard]] MirrorMode GetMirrorMode();

    void WritePatternTable( u16 addr, u8 data );
    void WriteNameTable( u16 addr, u8 data );
    u16  ResolveNameTableAddress( u16 addr );
    void Tick();

  private:
    // Debugging
    bool _isDisabled = false;

    // Timing
    s16  _scanline = 0;
    u16  _cycle = 4;
    u64  _lastSync = 0; // Last cpu cycle synced
    u64  _frame = 1;
    bool _isOddFrame = false;
    bool _isRenderingEnabled = false;
    bool _preventVBlank = false;
    bool _isCpuReadingPpuStatus = false;

    // CPU Memory Mappings
    u8  _ppuCtrl = 0x00;   // $2000
    u8  _ppuMask = 0x00;   // $2001
    u8  _ppuStatus = 0x00; // $2002
    u8  _oamAddr = 0x00;   // $2003
    u8  _oamData = 0x00;   // $2004
    u16 _ppuScroll = 0x00; // $2005
    u16 _ppuAddr = 0x00;   // $2006
    u8  _ppuData = 0x00;   // $2007

    // Internal temp register
    // This register helps the PPU with various operations
    u16 _tempAddr = 0x00;

    // Address latch
    // This is used to help break up operations that require two writes
    bool _addrLatch = false;

    // PPU Data Buffer
    // Holds data from the previous read
    u8 _dataBuffer = 0x00;

    // PPU Memory Mappings

    // Pattern Tables
    // $0000-$1FFF: Pattern Table 1
    // $1000-$1FFF: Pattern Table 2
    // Defined in cartridge.h as _char_rom / _chr_ram

    /* Name Table Memory
        The $2000 address for the PPU control register in the CPU's memory map
        is distinct from the $2000 address where the name tables begin in the PPU's address space.
        Name tables are part of the PPU's internal memory while PPU registers ($2000–$2007)
        are mapped into the CPU's memory space.

        Ranges:
        $2000-$23FF: Name Table 0 (physical VRAM)
        $2400-$27FF: Name Table 1 (physical VRAM)
        $2800-$2BFF: Name Table 2 (mapped via mirroring to physical VRAM)
        $2C00-$2FFF: Name Table 3 (mapped via mirroring to physical VRAM)
        Mirroring (horizontal, vertical, or custom) is controlled by the cartridge mapper.
        The PPU provides 2 KiB of VRAM to store two physical nametables.
    */
    array<u8, 2048> _nameTables{};
};
