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

class Bus;

class PPU
{
  public:
    explicit PPU( Bus *bus, bool isDisabled = false );

    // Getters
    [[nodiscard]] s16 GetScanline() const;
    [[nodiscard]] u16 GetCycles() const;
    [[nodiscard]] u8  GetControlFlag( ControlFlag flag ) const;

    // Setters
    void SetScanline( s16 scanline );
    void SetCycles( u16 cycles );
    void SetIsCpuReadingPpuStatus( bool isReading );

    // External Read / Write
    [[nodiscard]] u8 HandleCpuRead( u16 addr );
    void             HandleCpuWrite( u16 addr, u8 data );

    // Internal Read / Write
    [[nodiscard]] u8 Read( u16 addr );
    void             Write( u16 addr, u8 data );

    // PPU Methods
    [[nodiscard]] u8         ReadPatternTable( u16 addr );
    [[nodiscard]] u8         ReadNameTable( u16 addr );
    [[nodiscard]] MirrorMode GetMirrorMode();

    void DmaTransfer( u8 data );
    void WritePatternTable( u16 addr, u8 data );
    void WriteNameTable( u16 addr, u8 data );
    u16  ResolveNameTableAddress( u16 addr );
    void Tick();

  private:
    Bus *_bus;

    // Debugging
    bool _isDisabled = false;

    // Timing
    s16  _scanline = 0;
    u16  _cycle = 4;
    u64  _lastSync = 0; // Last cpu cycle synced
    u64  _frame = 1;
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

    // $4014: OAM DMA, handled in the write method

    // This register is just like the _tempAddr
    // It gets updated by the _tempAddr when the Vblank period ends
    // It holds state as follows:
    // Bits 0-4: Coarse x scroll
    // Bits 5-9: Coarse y scroll
    // Bits 10: Nametable X
    // Bits 11: Nametable Y
    // Bits 12-14: Fine y scroll
    // Bits 15: Unused
    u16 _vramAddr = 0x00;
    u16 _tempAddr = 0x00;

    // Internal fine X scroll register
    u8 _fineX = 0x00;

    // Address latch
    // This is used to help break up operations that require two writes
    bool _addrLatch = false;

    // PPU Data Buffer
    // Holds data from the previous read or write
    u8 _dataBuffer = 0x00;

    // PPU Memory Mappings

    // Pattern Tables
    // $0000-$0FFF: Pattern Table 1
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

    /*
       Palette Memory
      $3F00-$3F0F: Background Palettes
      $3F10-$3F1F: Sprite Palettes

      A palette is a group of 4 indices, each index ranging from 0-63
      The NES has 64 fixed colors, so each index represents a color

      The colors won't be defined here, but somewhere in the SDL
      rendering logic. We'll use .pal files to easily define and swap fixed colors
      It's worth documenting what the palettes are, as this concept can be confusing

      Background Palettes
      Palette 0: $3F00 (bg color), $3F01, $3F02, $3F03.
      Palette 1: $3F04 (bg color), $3F05, $3F06, $3F07.
      Palette 2: $3F08 (bg color), $3F09, $3F0A, $3F0B.
      Palette 3: $3F0C (bg color), $3F0D, $3F0E, $3F0F.

      Sprite Palettes
      Palette 4: $3F10 (mirrors 3F00), $3F11, $3F12, $3F13.
      Palette 5: $3F14 (mirrors 3F04), $3F15, $3F16, $3F17.
      Palette 6: $3F18 (mirrors 3F08), $3F19, $3F1A, $3F1B.
      Palette 7: $3F1C (mirrors 3F0C), $3F1D, $3F1E, $3F1F.

      Sprite backgrounds, despite being mirrored, are ignored and treated
      as transparent.
    */

    // Default boot palette, will get changed by the cartridge
    array<u8, 0x20> _paletteMemory = {
        0x09, 0x01, 0x00, 0x01, // bg1
        0x00, 0x02, 0x02, 0x0D, // bg2
        0x08, 0x10, 0x08, 0x24, // bg3
        0x00, 0x00, 0x04, 0x2C, // bg4
        0x09, 0x01, 0x34, 0x03, // sprite1
        0x00, 0x04, 0x00, 0x14, // sprite2
        0x08, 0x3A, 0x00, 0x02, // sprite3
        0x00, 0x20, 0x2C, 0x08  // sprite4
    };

    /* Object Attribute Memory (OAM)
       This is a 256 byte region internal to the PPU
       It holds metadata for up to 64 sprites, with each sprite taking up 4 bytes.

      Byte 0: Y position of the sprite
      Byte 1: Tile index number
        Bit 0: Bank (0 = $0000, 1 = $1000) of tiles
        Bits 1-7: Tile number of top of sprite, 0-254
      Byte 2: Attributes
        Bits 0-1: Palette (4 to 7) of the sprite
        Bits 2-4: Unused
        Bit 5: Priority (0 = in front of background, 1 = behind background)
        Bit 6: Flip sprite horizontally
        Bit 7: Flip sprite vertically
      Byte 3: X position of the sprite
    */
    array<u8, 256> _oam{};
};
