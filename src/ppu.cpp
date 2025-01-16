#include "ppu.h"
#include "mappers/mapper-base.h"

PPU::PPU( bool isDisabled ) : _isDisabled( isDisabled ) {}

[[nodiscard]] u16 PPU::GetScanline() const { return _scanline; }
[[nodiscard]] u16 PPU::GetCycle() const { return _cycle; }
[[nodiscard]] u8  PPU::GetControlFlag( ControlFlag flag ) const
{
    u8 const mask = 1 << flag;
    return static_cast<u8>( ( _ppuCtrl & mask ) > 0 );
}

[[nodiscard]] u8 PPU::HandleCpuRead( u16 address )
{
    /* @brief: CPU reads to the PPU
     * @details: Unlike other places in memory, doing reads here
     * has side effects.
     */

    // Non-readable registers: 2000 (PPUCTRL), 2001 (PPUMASK), 2003 (OAMADDR), 2005 (PPUSCROLL),
    // 2006 (PPUADDR)
    if ( _isDisabled || address == 0x2000 || address == 0x2001 || address == 0x2003 ||
         address == 0x2005 || address == 0x2006 )
    {
        return 0x00;
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

        return data;
    }

    // 2004: OAM Data
    if ( address == 0x2004 )
    {
        // TODO: Handle OAM read
        return 0x00;
    }

    // 2007: PPU Data
    if ( address == 0x2007 )
    {
        // TODO: Handle PPU Data read
        return 0x00;
    }

    return 0x00;
}

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
            u8 const nametable_x = GetControlFlag( ControlFlag::NametableX );
            u8 const nametable_y = GetControlFlag( ControlFlag::NametableY );

            // set nametableX to bit 10 of the temp address register, and nametableY to bit 11
            _tempAddr = ( _tempAddr & 0xF3FF ) | ( nametable_y << 11 ) | ( nametable_x << 10 );
            break;
        }

        case 0x2001:
            _ppuMask = data;
            break;
        case 0x2002: // Status, not writable
            break;
        case 0x2003:
            _oamAddr = data;
            break;
        case 0x2004: // NOLINT
            // TODO: Handle OAM Data write
            break;
        case 0x2005:
            // TODO: Handle Scroll write
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
                _tempAddr |= data;
                _ppuAddr = _tempAddr & 0x3FFF;
                _addrLatch = false;
                _tempAddr = 0x00;
            }
            break;
        case 0x2007:
            // TODO: Handle PPU Data write
            break;
        default:
            return;
    }
}

void PPU::Tick()
{
    if ( _isDisabled )
    {
        return;
    }

    // ---------------------------------------------
    // 1. Handle the special "skip dot" on odd frames
    //    Only applies if rendering is enabled.
    // ---------------------------------------------
    if ( _scanline == 261 && _cycle == 339 && _isOddFrame && _isRenderingEnabled )
    {
        _cycle = 0;
        _scanline = 0;
        _isOddFrame = !_isOddFrame;
        return;
    }

    _cycle++;
    // ---------------------------------------------
    // 2. If we've hit the end of a scanline (341 cycles),
    //    move to the next line, or wrap around to 0
    // ---------------------------------------------
    if ( _cycle > 340 )
    {
        _cycle = 0;
        _scanline++;

        if ( _scanline > 261 )
        {
            _scanline = 0;
            _isOddFrame = !_isOddFrame;
        }
    }

    // ---------------------------------------------
    // 3. Set VBlank flag
    //    Happens at scanline 241, cycle 1
    // ---------------------------------------------
    if ( _scanline == 241 && _cycle == 1 )
    {
        _ppuStatus |= Status::VerticalBlank;

        // TODO: Trigger NMI
    }

    // ---------------------------------------------
    // 4. Clear VBlank flag
    //    Happens at scanline 261, cycle 1 (start of pre-render)
    // ---------------------------------------------
    if ( _scanline == 261 && _cycle == 1 )
    {
        _ppuStatus &= ~Status::VerticalBlank;
        _frame++;
    }
    // ---------------------------------------------
    // 5. (Placeholder) Additional PPU logic
    //    - Background fetches
    //    - Sprite evaluation
    //    - Checking for sprite 0 hits
    //    - Incrementing horizontal/vertical scroll bits
    //    etc.
    // ---------------------------------------------
}

void PPU::SyncPPU( u64 current_cpu_cycles )
{
    // Edge where the cpu cycle overflows
    // just kidding, that would take lifetimes for a 64 bit int..

    if ( _isDisabled )
    {
        return;
    }

    u64 const cycles_since_sync = current_cpu_cycles - _lastSync;
    u64 const ppu_cycles = cycles_since_sync * 3;
    for ( u64 i = 0; i < ppu_cycles; i++ )
    {
        Tick();
    }
    _lastSync = current_cpu_cycles;
}
