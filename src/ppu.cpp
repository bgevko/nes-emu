#include "ppu.h"
#include "bus.h"
#include "cartridge.h"
#include "mappers/mapper-base.h"

PPU::PPU( Bus *bus, bool isDisabled ) : _bus( bus ), _isDisabled( isDisabled ) {}

/*
################################
||                            ||
||      Getters / Setters     ||
||                            ||
################################
*/
[[nodiscard]] s16 PPU::GetScanline() const { return _scanline; }
[[nodiscard]] u16 PPU::GetCycles() const { return _cycle; }
[[nodiscard]] u8  PPU::GetControlFlag( ControlFlag flag ) const
{
    u8 const mask = 1 << flag;
    return static_cast<u8>( ( _ppuCtrl & mask ) > 0 );
}
void PPU::SetScanline( s16 scanline ) { _scanline = scanline; }
void PPU::SetCycles( u16 cycles ) { _cycle = cycles; }
void PPU::SetIsCpuReadingPpuStatus( bool isReading ) { _isCpuReadingPpuStatus = isReading; }

/*
################################
||                            ||
||       Handle CPU Read      ||
||                            ||
################################
*/
[[nodiscard]] u8 PPU::HandleCpuRead( u16 address )
{
    /* @brief: CPU reads to the PPU
     * @details: Unlike other places in memory, doing reads here
     * has side effects.
     */

    // Non-readable registers: 2000 (PPUCTRL), 2001 (PPUMASK), 2003 (OAMADDR), 2005 (PPUSCROLL),
    // 2006 (PPUADDR)
    if ( _isDisabled || address == 0x2000 || address == 0x2001 || address == 0x2003 || address == 0x2005 ||
         address == 0x2006 )
    {
        return 0xFF;
    }

    // 2002: PPU Status Register
    if ( address == 0x2002 )
    {
        /* @brief PPUSTATUS ($2002) Read Behavior
         *  @details
         *  Returns:
         *   - Bits 7-5: Current PPU status flags.
         *     - Bit 7: Vertical blank flag (reflects state before the read).
         *     - Bit 6: Sprite 0 hit flag.
         *     - Bit 5: Sprite overflow flag.
         *   - Bits 4-0: Last VRAM bus transaction (ppuDataBuffer).

         * Side Effects of Reading:
         *   1. Clears the vertical blank flag (Bit 7).
         *   2. Resets the PPU address latch for $2005/$2006.

         * The CPU expects the state of the PPU at the time of the read. The vertical
         * blank flag is cleared after constructing the return value, ensuring the
         * CPU sees the state before the read's side effects.
      */

        // Status grabs the top 3 bits of the status register
        u8 const status = _ppuStatus & 0xE0;

        // The ppu data buffer has remnant data from the last read, which some games use
        u8 const noise = _dataBuffer & 0x1F;

        // The CPU expects the state of the PPU at the time of the read
        // the read does have a side effect of clearing the VerticalBlank flag, but
        // this is not reflected in the data returned by the read
        u8 const data = status | noise;

        // Clear the vertical blank flag
        _ppuStatus &= ~Status::VerticalBlank;

        // Reset the address latch
        _addrLatch = false;

        _isCpuReadingPpuStatus = false;
        _preventVBlank = false;
        _dataBuffer = data;
        return data;
    }

    // 2004: OAM Data
    if ( address == 0x2004 )
    {
        if ( _isRenderingEnabled && _scanline >= 0 && _scanline < 240 )
        {
            // Not not supposed to read OAM data during rendering, or it will cause visual glitches
            // Real hardware return corrunted data
            return 0xFF;
        }

        // Return the oam data at the current oam address, (held in register 2003)
        return _oam[_oamAddr];
    }

    // 2007: PPU Data
    if ( address == 0x2007 )
    {
        u8 data = 0x00;

        // If accessing palette memory ($3F00-$3FFF), use direct read
        if ( _vramAddr >= 0x3F00 && _vramAddr <= 0x3FFF )
        {
            data = Read( _vramAddr );
        }
        else
        {
            // Buffered read for VRAM, delayed by one cycle
            data = _dataBuffer;                       // Return buffered value
            _dataBuffer = Read( _vramAddr & 0x3FFF ); // Fetch next value
        }

        // Increment VRAM address based on $2000 settings
        _vramAddr += ( GetControlFlag( ControlFlag::VramIncrementMode ) != 0 ? 32 : 1 );
        return data;
    }

    return 0xFF;
}

/*
################################
||                            ||
||      Handle CPU Write      ||
||                            ||
################################
*/
void PPU::HandleCpuWrite( u16 address, u8 data )
{
    /* @brief: CPU writes to the PPU
     */

    if ( _isDisabled )
    {
        return;
    }

    switch ( address )
    {
        case 0x2000:
        {
            _ppuCtrl = data;

            // Trigger NMI if NMI is enabled and VBlank is set
            // This is to ensure NMI can be called any time during the vblank period, not just
            // on the first cycle
            if ( _ppuCtrl & ControlFlag::NmiEnable && ( _ppuStatus & Status::VerticalBlank ) )
            {
                _bus->cpu.NMI();
            }

            u8 const nametable_x = GetControlFlag( ControlFlag::NametableX );
            u8 const nametable_y = GetControlFlag( ControlFlag::NametableY );

            // set nametableX to bit 10 of the temp address register, and nametableY to bit 11
            _tempAddr = ( _tempAddr & 0xF3FF ) | ( nametable_y << 11 ) | ( nametable_x << 10 );
            break;
        }

        case 0x2001:
        {
            // Bits 3 = BG enable, 4 = Sprite enable
            bool bg_enabled = ( data & 0x08 ) != 0;
            bool sprite_enabled = ( data & 0x10 ) != 0;
            _isRenderingEnabled = bg_enabled || sprite_enabled;
            _ppuMask = data;
            break;
        }
        case 0x2002: // Status, not writable
            break;
        case 0x2003:
            _oamAddr = data;
            break;
        case 0x2004:
            // OAM Data write
            // The NES allows writes to the OAMDATA, NES dev says that most of the time,
            // DMA transfer is used instead of writing here directly, as it's too slow. However,
            // the functionality exists, so we'll emulate it.
            // Writes during rendering do have an effect, but the docs recommend ignoring it for emulation
            if ( _isRenderingEnabled && ( _scanline < 240 || _scanline == -1 ) )
            {
                return;
            }
            // Write to OAM
            _oam[_oamAddr] = data;

            // Increment OAM address
            _oamAddr = ( _oamAddr + 1 ) & 0xFF;
            break;
        case 0x2005:
            /*
               Scrolling
               Fine scroll moves within a tile, values range from 0 to 7
               Coarse scroll moves 8 pixels at a time
            */
            if ( !_addrLatch )
            {
                // First write contains X offset
                // First, we store the coarse X in bits 0-4 in the temp address register
                u8 const coarse_x = data >> 3;
                _tempAddr = ( _tempAddr & 0xFFE0 ) | coarse_x;

                // Fine x is the first three bits of data
                // We'll store it in the temp fineX register
                u8 const fine_x = data & 0x07;
                _fineX = fine_x;

                _addrLatch = true;
            }
            else
            {
                // Second write

                // Fine y is the first 3 bits of the data
                // We'll push it to bit position 12-14, as that's where it is stored in the temp address
                // register
                u16 const fine_y = ( ( data & 0x07 ) << 12 );
                _tempAddr = ( _tempAddr & 0x8FFF ) | fine_y;

                // Coarse y is the next 5 bits of the data
                // We'll slide it left 2 bits to slot into bits 5-9 of the temp address register
                u16 const coarse_y = ( data & 0xF8 ) << 2;
                _tempAddr = ( _tempAddr & 0xFC1F ) | coarse_y;

                _addrLatch = false;
            }
            break;
        case 0x2006:
            if ( !_addrLatch )
            {
                // First write
                // Read in the high byte into the temp address
                _tempAddr = data << 8;
                _addrLatch = true;
            }
            else
            {
                // Second write
                // Combine with the low byte, and mask to 14 bits
                // PPU address dosn't use bits 14 and 15
                _tempAddr = ( _tempAddr & 0xFF00 ) | data;
                _vramAddr = _tempAddr & 0x3FFF;
                _addrLatch = false;
            }
            break;
        case 0x2007:
        {
            Write( _vramAddr, data );
            _vramAddr += ( GetControlFlag( ControlFlag::VramIncrementMode ) != 0 ? 32 : 1 );
            break;
        }

        default:
            return;
    }
}

/*
################################
||                            ||
||           OAM DMA          ||
||                            ||
################################
*/
void PPU::DmaTransfer( u8 data )
{
    /* @details CPU writes to address $4014 to initiate a DMA transfer
     * DMA transfer is a way to quickly transfer data from CPU memory to the OAM.
     * The CPU sends the starting address, N, to $4014, which is the high byte of the
     * source address
     * i.e. 0x02 -> 0x0200, 0x03 -> 0x0300, etc.
     *
     * Once the DMA transfer starts, the CPU reads 256 bytes sequentally from the address
     * into the OAM. The CPU is halted for 513/514 cycles during this time. Games usually
     * trigger this during a Vblank period
     *
     * Practically speaking, updating the OAM means updating the sprite information on screen
     *
     * This is not the only way to update the OAM, registers 2004 and 2003 can be used
     * but those are slower, and are used for partial updates mostly
     */

    u16 const address = data << 8;
    for ( u16 i = 0; i < 256; i++ )
    {
        _oam[i] = _bus->Read( address + i );
    }
}
/*
################################
||                            ||
||          PPU Read          ||
||                            ||
################################
*/

[[nodiscard]] u8 PPU::Read( u16 address )
{
    /*@brief: Internal PPU reads to the cartridge
     */

    // $0000-$1FFF: Pattern Tables
    if ( address >= 0x0000 && address <= 0x1FFF )
    {
        return _bus->cartridge->Read( address );
    }

    // $2000-$3EFF: Name Tables
    if ( address >= 0x2000 && address <= 0x3EFF )
    {
        // Address 3000-3EFF is not not valid and can be mirrored to 2000-2EFF
        address &= 0x2FFF;

        // In four screen mode, there are 4 unique name tables
        // The PPU only has 2KiB of vram, so the other 2KiB is stored in the cartridge
        if ( _bus->cartridge->GetMirrorMode() == MirrorMode::FourScreen && address >= 0x2800 )
        {
            return _bus->cartridge->ReadCartridgeVRAM( address );
        }

        u16 nametable_addr = ResolveNameTableAddress( address );

        return _nameTables[nametable_addr];
    }

    // $3F00-$3FFF: Palettes
    if ( address >= 0x3F00 && address <= 0x3FFF )
    {
        /*
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
        */
        address &= 0x001F; // Mask to 32 bytes

        // Mirror backgrounds of palettes 4-7 to 0-3
        if ( address >= 0x0010 && ( address & 0x0003 ) == 0 )
        {
            address -= 0x0010;
        }

        // Return as a 6 bit value (0-63)
        return _paletteMemory[address] & 0x3F;
    }

    return 0xFF;
}

/*
################################
||                            ||
||          PPU Write         ||
||                            ||
################################
*/
void PPU::Write( u16 address, u8 data )
{
    /*@brief: Internal PPU reads to the cartridge
     */

    address &= 0x3FFF;

    if ( address >= 0x0000 && address <= 0x1FFF )
    {
        // Write to Pattern table memory
        _bus->cartridge->WriteChrRAM( address, data );
        return;
    }
    if ( address >= 0x2000 && address <= 0x2FFF )
    {
        // Address 3000-3EFF is not not valid and can be mirrored to 2000-2EFF
        address &= 0x2FFF;

        // In four screen mode, there are 4 unique name tables
        // The PPU only has 2KiB of vram, so the other 2KiB is stored in the cartridge
        if ( _bus->cartridge->GetMirrorMode() == MirrorMode::FourScreen && address >= 0x2800 )
        {
            _bus->cartridge->WriteCartridgeVRAM( address, data );
            return;
        }
        u16 nametable_addr = ResolveNameTableAddress( address );
        _nameTables[nametable_addr] = data;
        return;
    }

    // $3F00-$3FFF: Palettes
    if ( address >= 0x3F00 && address <= 0x3FFF )
    {
        address &= 0x001F; // Mask to 32 bytes

        // Mirror backgrounds of palettes 4-7 to 0-3
        if ( address >= 0x0010 && ( address & 0x0003 ) == 0 )
        {
            address -= 0x0010;
        }

        _paletteMemory[address] = data;
    }
}

/*
################################
||                            ||
||       PPU Cycle Tick       ||
||                            ||
################################
*/
void PPU::Tick()
{
    bool is_odd_frame = _frame % 2 == 1;
    if ( _isDisabled )
    {
        return;
    }
    // 1. Handle the odd-frame skip dot (when rendering is enabled)
    //    Happens at scanline = -1, cycle = 339.
    if ( _scanline == -1 && _cycle == 339 && is_odd_frame && _isRenderingEnabled )
    {
        _cycle = 0;
        _scanline = 0;
        return;
    }

    _cycle++;

    // 2. Check if reached end of scanline
    // (NTSC has 341 PPU cycles per line)
    if ( _cycle > 340 )
    {
        _cycle = 0;
        _scanline++;

        // After scanline 260, wrap around to -1 (pre-render line)
        if ( _scanline > 260 )
        {
            _scanline = -1;
            _frame++;
        }
    }

    // if we happen to do a read on scanline 241, cycle 0, Vblank
    // Doesn't get set. It's a hardware quirk. We can simulate it by just
    // turning it off here
    if ( _scanline == 241 && _cycle == 0 && _isCpuReadingPpuStatus )
    {
        _preventVBlank = true;
    }

    // 3. Set Vblank flag on scanline 241, cycle 1
    if ( _scanline == 241 && _cycle == 1 )
    {
        if ( !_preventVBlank )
        {
            _ppuStatus |= Status::VerticalBlank;

            // Trigger NMI if NMI is enabled
            if ( _ppuCtrl & ControlFlag::NmiEnable )
            {
                _bus->cpu.NMI();
            }
        }
        _preventVBlank = false;
    }

    // 4. Clear VBlank flag at the start of the pre-render line
    //    (scanline = -1, cycle = 1)
    if ( _scanline == -1 && _cycle == 1 )
    {
        _ppuStatus &= ~Status::VerticalBlank;

        // copy temp address to VRAM address
        _vramAddr = _tempAddr;
    }

    // 5. Additional PPU logic (placeholder)
    //    - Background fetches
    //    - Sprite evaluation
    //    - Sprite 0 hit checks
    //    - Scrolling increments
    //    etc.
}

/*
################################
||                            ||
||           Helpers          ||
||                            ||
################################
*/

u16 PPU::ResolveNameTableAddress( u16 addr )
{
    MirrorMode mirror_mode = _bus->cartridge->GetMirrorMode();

    switch ( mirror_mode )
    {
        case MirrorMode::SingleUpper:
            // All addresses fall within 2000-23FF, nametable 0
            return addr & 0x03FF;
        case MirrorMode::SingleLower:
            // All addresses fall within 2800-2BFF, nametable 2
            return ( addr & 0x03FF ) + 0x800;
        case MirrorMode::Vertical:
            /* Vertical Mirroring
              The two horizontal sections are unique, but the two vertical sections are mirrored
              2800-2FFF is a mirror of 2000-27FF

              2000 2400
              ^    ^
              v    v
              2800 2C00

              Horizontal scrolling games will use this mode. When screen data exceeds 27FF, it's
              wrapped back to 2000.
             */

            return addr & 0x07FF; // 0000-07FF local (2000-27FF actual)
        case MirrorMode::Horizontal:
            /* Horizontal Mirroring
              The two vertical sections are unique, but the two horizontal sections are mirrored
              2400-27FF is a mirror of 2000-23FF
              2C00-2FFF is a mirror of 2800-2BFF

              2000 < > 2400
              2800 < > 2C00

              Horizontal mode is used for vertical scrolling games, like Kid Icarus.
             */
            if ( addr >= 0x2800 )
            {
                return ( addr & 0x03FF ) + 0x800; // 0800-0BFF local (2800-2BFF actual)
            }
            return addr & 0x03FF; // 0000-03FF local (2000-23FF actual)
        case MirrorMode::FourScreen:
            /* Four-Screen Mirroring
               All four nametables are unique and backed by cartridge VRAM. There's no mirroring.
             */
            return addr & 0x0FFF;
        default:
            // Default to vertical mirroring
            return addr & 0x07FF;
    }
    return 0xFF;
}
