#include "ppu.h"
#include "bus.h"
#include "utils.h"
#include "cartridge.h" // NOLINT
#include "mappers/mapper-base.h"
#include <exception>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <stdexcept>

PPU::PPU( Bus *bus ) : _bus( bus )
{
    try {
        _nesPaletteRgbValues = utils::readPalette( "palettes/palette1.pal" );
    } catch ( std::exception &e ) {
        exit( EXIT_FAILURE );
    }
}

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
         address == 0x2006 ) {
        return 0xFF;
    }

    // 2002: PPU Status Register
    if ( address == 0x2002 ) {
        /* @brief PPUSTATUS ($2002) Read Behavior

         * Side Effects of Reading:
         *   1. Clears the vertical blank flag (Bit 7).
         *   2. Resets the PPU address latch for $2005/$2006.

         * The CPU expects the state of the PPU at the time of the read. The vertical
         * blank flag is cleared after constructing the return value, ensuring the
         * CPU sees the state before the read's side effects.
      */

        u8 const status = _ppuStatus.value & 0xE0;
        u8 const noise = _dataBuffer & 0x1F;
        u8 const data = status | noise;
        _ppuStatus.bit.verticalBlank = 0;
        _addrLatch = false;
        _bus->cpu.SetReading2002( false );
        _preventVBlank = false;
        return data;
    }

    // 2004: OAM Data
    if ( address == 0x2004 ) {
        /*
          Return the contents of _oam[_oamaddr]
          If called while _renderingEnabled and in visible scanline range (0-239),
          Returns corrupted data (0xFF)
        */
        if ( _isRenderingEnabled && _scanline >= 0 && _scanline < 240 ) {
            return 0xFF;
        }

        return _oam.at( _oamAddr & 0xFF );
    }

    // 2007: PPU Data
    if ( address == 0x2007 ) {
        u8 data = 0x00;

        // If accessing palette memory ($3F00-$3FFF), use direct read
        if ( _vramAddr.value >= 0x3F00 && _vramAddr.value <= 0x3FFF ) {
            data = Read( _vramAddr.value );
        } else {
            // Buffered read for VRAM, delayed by one cycle
            data = _dataBuffer;                             // Return buffered value
            _dataBuffer = Read( _vramAddr.value & 0x3FFF ); // Fetch next value
        }

        // Increment VRAM address based on ppuctrl increment mode
        _vramAddr.value += _ppuCtrl.bit.vramIncrement ? 32 : 0;
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

    if ( _isDisabled ) {
        return;
    }

    switch ( address ) {
        case 0x2000: {
            _ppuCtrl.value = data;

            // Trigger NMI if NMI is enabled and VBlank is set
            // This is to ensure NMI can be called any time during the vblank period, not just
            // on the first cycle

            if ( _ppuCtrl.bit.nmiEnable && _ppuStatus.bit.verticalBlank ) {
                TriggerNmi();
            }

            // Copy bits 0-1 of ppuctrl to the temp address register,
            // which are the nametable x and y values
            _tempAddr.bit.nametableX = _ppuCtrl.bit.nametableX;
            _tempAddr.bit.nametableY = _ppuCtrl.bit.nametableY;
            break;
        }

        case 0x2001: {
            // Bits 3 = BG enable, 4 = Sprite enable
            _ppuMask.value = data;
            _isRenderingEnabled = _ppuMask.bit.renderBackground || _ppuMask.bit.renderSprites;
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
            if ( _isRenderingEnabled && ( _scanline < 240 || _scanline == -1 ) ) {
                return;
            }
            // Write to OAM
            _oam.at( _oamAddr & 0xFF ) = data;

            // Increment OAM address
            _oamAddr = ( _oamAddr + 1 ) & 0xFF;
            break;
        case 0x2005:
            /*
               Scrolling
               Fine scroll moves within a tile, values range from 0 to 7
               Coarse scroll moves 8 pixels at a time
            */
            if ( !_addrLatch ) {
                // First write contains X offset
                // Bits 3-7 from data are the coarse x scroll, stored in the temp register
                _tempAddr.bit.coarseX = ( data & 0b11111000 ) >> 3;

                // Bits 0-2 of data are the fine x scroll, stored in the fine x register
                _fineX = data & 0b00000111;

                _addrLatch = true;
            } else {
                // Second write

                // Fine y is the first 3 bits of the data
                _tempAddr.bit.fineY = data & 0b00000111;

                // Coarse y is the next 5 bits of the data
                _tempAddr.bit.coarseY = ( data & 0b11111000 ) >> 3;

                _addrLatch = false;
            }
            break;
        case 0x2006:
            // PPU Address
            if ( !_addrLatch ) {
                // First write
                // First 6 bits of data are set to bits 8-14 of the temp address
                // bit 14 of the temp address is set to 0
                u16 const high = ( data &= 0b00111111 ) << 8;
                _tempAddr.value = ( _tempAddr.value & 0x00FF ) | high;
                _addrLatch = true;
            } else {
                // Second write
                // All 8 bits of data are set to the lower 8 bits of the temp address
                _tempAddr.value = ( _tempAddr.value & 0xFF00 ) | data;

                // temp address is copied to vram address
                _vramAddr.value = _tempAddr.value;
                _addrLatch = false;
            }
            break;
        case 0x2007: {
            Write( _vramAddr.value, data );
            _vramAddr.value += _ppuCtrl.bit.vramIncrement == 0 ? 1 : 32;
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
void PPU::DmaTransfer( u8 data ) // NOLINT
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
    if ( _bus == nullptr ) {
        return;
    }

    u16 const address = data << 8;
    for ( u16 i = 0; i < 256; i++ ) {
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

    if ( _bus->cartridge == nullptr ) {
        return 0xFF;
    }

    // $0000-$1FFF: Pattern Tables
    if ( address >= 0x0000 && address <= 0x1FFF ) {
        return _bus->cartridge->Read( address );
    }

    // $2000-$3EFF: Name Tables
    if ( address >= 0x2000 && address <= 0x3EFF ) {
        // Address 3000-3EFF is not not valid and can be mirrored to 2000-2EFF
        address &= 0x2FFF;

        // In four screen mode, there are 4 unique name tables
        // The PPU only has 2KiB of vram, so the other 2KiB is stored in the cartridge
        if ( _bus->cartridge->GetMirrorMode() == MirrorMode::FourScreen && address >= 0x2800 ) {
            return _bus->cartridge->ReadCartridgeVRAM( address );
        }

        u16 const nametableAddr = ResolveNameTableAddress( address );

        return _nameTables.at( nametableAddr & 0x07FF );
    }

    // $3F00-$3FFF: Palettes
    if ( address >= 0x3F00 && address <= 0x3FFF ) {
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
        if ( address >= 0x0010 && ( address & 0x0003 ) == 0 ) {
            address -= 0x0010;
        }

        // Return as a 6 bit value (0-63)
        return _paletteMemory.at( address ) & 0x3F;
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
    if ( _bus->cartridge == nullptr ) {
        return;
    }

    address &= 0x3FFF;

    if ( address >= 0x0000 && address <= 0x1FFF ) {
        // Write to Pattern table memory
        _bus->cartridge->Write( address, data );
        return;
    }
    if ( address >= 0x2000 && address <= 0x2FFF ) {
        // Address 3000-3EFF is not not valid and can be mirrored to 2000-2EFF
        address &= 0x2FFF;

        // In four screen mode, there are 4 unique name tables
        // The PPU only has 2KiB of vram, so the other 2KiB is stored in the cartridge
        if ( _bus->cartridge->GetMirrorMode() == MirrorMode::FourScreen && address >= 0x2800 ) {
            _bus->cartridge->WriteCartridgeVRAM( address, data );
            return;
        }
        u16 const nametableAddr = ResolveNameTableAddress( address );
        _nameTables.at( nametableAddr & 0x07FF ) = data;
        return;
    }

    // $3F00-$3FFF: Palettes
    if ( address >= 0x3F00 && address <= 0x3FFF ) {
        address &= 0x001F; // Mask to 32 bytes

        // Mirror backgrounds of palettes 4-7 to 0-3
        if ( address >= 0x0010 && ( address & 0x0003 ) == 0 ) {
            address -= 0x0010;
        }

        _paletteMemory.at( address & 0x1F ) = data;
    }
}

/*
################################
||                            ||
||       PPU Cycle Tick       ||
||                            ||
################################
*/
void PPU::Tick() // NOLINT
{
    if ( _isDisabled ) {
        return;
    }

    /*
    ################################
    ||                            ||
    ||  Pre-render Scanline (-1)  ||
    ||                            ||
    ################################
    */

    if ( _scanline == -1 ) {

        /*
        ################################
        ||    Odd Frame Cycle Skip    ||
        ################################
        */
        bool const isOddFrame = ( _frame & 1 ) == 1;
        if ( _scanline == -1 && _cycle == 339 && isOddFrame && _isRenderingEnabled ) {
            _cycle = 0;
            _scanline = 0;
            return;
        }

        /*
        ################################
        ||         Vblank End         ||
        ################################
        */
        if ( _scanline == -1 && _cycle == 1 ) {
            _ppuStatus.bit.verticalBlank = 0;
            _ppuStatus.bit.spriteOverflow = 0;
            _ppuStatus.bit.spriteZeroHit = 0;

            // TODO: Clear sprite shifters
        }

        /*
        ################################
        ||    Transfer Y (280-340)    ||
        ################################
        */
        if ( _scanline == -1 && _cycle >= 280 && _cycle <= 304 ) {
            _vramAddr.bit.nametableY = _tempAddr.bit.nametableY;
            _vramAddr.bit.coarseY = _tempAddr.bit.coarseY;
            _vramAddr.bit.fineY = _tempAddr.bit.fineY;
        }
    }

    /*
    #################################
    ||                             ||
    ||  Visible Scanlines (0-239)  ||
    ||                             ||
    #################################
    */
    if ( _scanline >= 0 && _scanline <= 239 ) {

        // Cycle 0, Idle cycle

        /*
        #########################################
        ||  Background Events, every 8 cycles  ||
        #########################################
        */
        // Cycles 1-256: Tile and Pixel Rendering
        // Cycles 321-336 are beyond the visible scanline, but continue for the next scanline
        if ( ( _cycle >= 1 && _cycle <= 256 ) || ( _cycle >= 321 && _cycle <= 336 ) ) {

            // Update the current pixel info
            UpdateShiftRegisters();

            /* Rendering events
               PPU does a series of events which repeat every 8 cycles
               8 cycles is 8 pixels, which is the width of a tile
               See the wiki PPU diagram for more details
               https://www.nesdev.org/w/images/default/4/4f/Ppu.svg
             */
            u8 const event = ( _cycle - 1 ) & 0x07;

            switch ( event ) {
                // 0-1 fetch the nametable byte
                case 1: {

                    LoadNextBgShiftRegisters();
                    LoadNametableByte();
                    break;
                }

                // 2-3 fetch the attribute table byte
                case 3: {
                    LoadAttributeByte();
                    break;
                }
                // 4-5 fetch pattern table plane 0
                case 5: {
                    LoadPatternPlane0Byte();
                    break;
                }
                // 6-7 fetch pattern table plane 1
                // 7: Increment scroll x
                // 7: Increment scroll y on cycle 256
                case 7: {
                    LoadPatternPlane1Byte();
                    IncrementScrollX();
                    IncrementScrollY();
                    break;
                }
                default:
                    break;
            }
        }

        /*
        ##################################
        ||  End of Tile Fetching (257)  ||
        ##################################
        */
        if ( _cycle == 257 ) {
            LoadNextBgShiftRegisters();

            // Transfer nametable and coarse X from temp to vram address
            _vramAddr.bit.nametableX = _tempAddr.bit.nametableX;
            _vramAddr.bit.coarseX = _tempAddr.bit.coarseX;
        }

        /*
        ###################################
        ||                               ||
        ||  Sprite Evaluation (257-320)  ||
        ||                               ||
        ###################################
        */
        if ( _cycle >= 257 && _cycle <= 320 ) {
            //  TODO: Sprite evaluation
        }

        /*
        ################################
        ||   Unused Reads (338, 340)  ||
        ################################
        */
        if ( _cycle == 338 || _cycle == 340 ) {
            _nametableByte = Read( 0x2000 | ( _vramAddr.value & 0x0FFF ) );
        }
    }

    /*
    ################################
    ||        Vblank Start        ||
    ################################
    */
    if ( _scanline == 241 ) {
        // If the CPU is reading register 2002 on cycle 0 of scanline 241
        // Vblank will not be set until the next frame due to a hardware race condition bug
        if ( _cycle == 0 && _bus->cpu.IsReading2002() ) {
            _preventVBlank = true;
        }

        // Set the Vblank flag on cycle 1
        if ( _cycle == 1 ) {

            // SDL Callback for rendering the framebuffer
            if ( onFrameReady != nullptr ) {
                onFrameReady( _frameBuffer.data() );
            }

            // Vblank and NMI
            if ( !_preventVBlank ) {
                _ppuStatus.bit.verticalBlank = 1;

                // Trigger NMI if NMI is enabled
                if ( _ppuCtrl.bit.nmiEnable ) {
                    TriggerNmi();
                }
            }
            _preventVBlank = false;
        }
    }

    /*
    ################################
    ||                            ||
    ||           Drawing          ||
    ||                            ||
    ################################
    */
    if ( _scanline >= 0 && _scanline < 240 && _cycle >= 0 && _cycle < 256 ) {
        u8 const  bgPalette = GetBgPalette();
        u8 const  spritePalette = GetSpritePalette();
        u8 const  bgPixel = GetBgPixel();
        u8 const  spritePixel = GetSpritePixel();
        u32 const outputPixel = GetOutputPixel( bgPixel, spritePixel, bgPalette, spritePalette );
        u16 const bufferIndex = ( _scanline * 256 ) + _cycle;

        try {
            _frameBuffer.at( bufferIndex ) = outputPixel;
        } catch ( std::out_of_range &e ) {
            std::cerr << e.what() << '\n';
            std::cerr << "Buffer Index: " << bufferIndex << '\n';
            std::cerr << "Scanline: " << _scanline << '\n';
            std::cerr << "Cycle: " << _cycle << '\n';
        }
    }

    _cycle++;

    /*
    ################################
    ||       End Of Scanline      ||
    ################################
    */
    if ( _cycle > 340 ) {
        _cycle = 0;
        _scanline++;

        if ( _scanline > 260 ) {
            _scanline = -1;
            _frame++;
        }
    }
}

/*
################################
||                            ||
||           Helpers          ||
||                            ||
################################
*/

void PPU::TriggerNmi()
{
    if ( _bus == nullptr ) {
        return;
    }
    if ( !_bus->cpu.IsNmiInProgress() ) {
        _bus->cpu.NMI();
    }
}

u16 PPU::ResolveNameTableAddress( u16 addr )
{
    /* @brief: Resolve the name table address according to the mirror mode, defined in the cartridge.
       @details:
       The PPU can have up to 4 nametables, which are 1 KiB each. Most cartridges only use 2.
       The mapper determines how the nametables are mirrored. Addresses to this helper will be
       between 2000-2FFF, and most will be converted to a value between 0000-07FF, which is 2048 bytes,
       the exact size of the _nameTables array. In cases 4 screen mode is used. This function can
       return addresses between 0000 and 0FFF, but in reality, addresses bigger than 27FF are intercepted
       by the cartridge before they make it here. 4 Screen mode will return an unmirrored part of the
       address space that's lower than 27FF, and the cartridge will handle the higher part.
     */
    if ( _bus == nullptr ) {
        return 0x00;
    }

    MirrorMode const mirrorMode = _bus->cartridge->GetMirrorMode();

    switch ( mirrorMode ) {
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
            if ( addr >= 0x2800 ) {
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

void PPU::UpdateShiftRegisters()
{
    /*
      @brief: Update the shift registers.
      @details: From an emulation perspective, the shift register holds the current pixel
      data that is ready to be turned into an RGB value.
    */

    /*
    ###########################
    ||  Background Shifters  ||
    ###########################

    The top 8 bits of the shift register hold the current tile data,
    The bottom 8 bits hold the next tile data.

    In essence, the shift registers (when combined) form the information for the
    current pixel on the screen, plus whatever the fine x offset is.

    The combined output might be represented as

    01221230 01122110
    ^

    The numbers are palette indeces, which will grab a color from the palette that
    the attribute table specifies. The ^ part is the pixel that is getting rendered,
    assuming fine x is 0. The lower 8 bits are the next tile, which is preloaded to
    allow the shift registers to move left by 1 pixel every cycle.

    The fine x value determines which pixel it is. So if fine x is 2, then the pixel output for
    this cycle would be:

    01221230 01122110
      ^

    Why do we need the lower 8 bits, 0112110? To understand, suppose we didn't have it.
    The shift each cycle might look like

    01221230 -> 1221230x -> 221230xx -> 21230xxx -> 1230xxxx ... -> xxxxxxxx

    And with the lower 8 bits:
    01221230 -> 12212300 -> 22123001 -> 21230011 -> 12300112 ... -> 01122110

    Without the next tile information, the PPU wouldn't know where to scroll to, as
    the left shift would just erase any data in the lower bits. With the next tile
    info, there's always valid data in index 0-7 of the current shift register

    Shift registers do not hold the final output ready for SDL. This output
    needs an additional step to convert palette indeces to RGB values
   */
    _bgShiftPatternLow <<= 1;
    _bgShiftPatternHigh <<= 1;
    _bgShiftAttributeLow <<= 1;
    _bgShiftAttributeHigh <<= 1;

    /*
    ################################
    ||       Sprite Shifters      ||
    ################################
    */
    if ( _ppuMask.bit.renderSprites && _cycle >= 1 && _cycle <= 256 ) {
        // TODO: Sprite shifters
    }
}

void PPU::LoadNextBgShiftRegisters()
{
    /* @brief Loads the next background tile information (lower 8 bits) into the background shifters
     */

    /* Update the background shift registers
       These are two 16-bit registers that hold the pattern table data for the current tile
       and the next tile. It preloads the tile data necessary for pixel-based scrolling
       The tops 8 bits hold the current tile, the bottom 8 bits hold the next tile
     */
    _bgShiftPatternLow = ( _bgShiftPatternLow & 0xFF00 ) | _bgPlane0Byte;
    _bgShiftPatternHigh = ( _bgShiftPatternHigh & 0xFF00 ) | _bgPlane1Byte;

    /* Update the attribute shift registers
       These are two 16-bit registers that hold the attribute table data (palette)
       for the current tile
       The tops 8 bits hold the current tile, the bottom 8 bits hold the next tile
       Since the attribute table is split into 4 regions, we can determine which
       region to mask based on the attribute table byte
     */
    u8 const lowMask = ( _attributeByte & 0b01 ) ? 0xFF : 0x00;
    u8 const highMask = ( _attributeByte & 0b10 ) ? 0xFF : 0x00;
    _bgShiftAttributeLow = ( _bgShiftAttributeLow & 0xFF00 ) | lowMask;
    _bgShiftAttributeHigh = ( _bgShiftAttributeHigh & 0xFF00 ) | highMask;
}

void PPU::LoadNametableByte()
{
    /*  @brief: Loads the current nametable byte from VRAM into _nametableByte
        @details: Nametable is a 1 KiB region that stores background tile indeces.
        in other words, it's the layout of the background tiles on the screen. This function
        sets the _nametableByte to the tile that the vram address is pointing to. In essence,
        it's telling the PPU which tile will appear in the background, but not the pixel data.
        The pixel data is acquired from the pattern table, which contains indices 0-3. The actual
        palette is in the attribute table, which defines what 0-3 will look like.
     */

    // Grab the first 12 bits of the VRAM address
    // which provide nametable select, coarse Y scroll, and coarse x scroll information
    // Nametable address is 0x2000 plus the offset of the vram address.
    _nametableByte = Read( 0x2000 | ( _vramAddr.value & 0x0FFF ) );
}

void PPU::LoadAttributeByte()
{

    /* Attribute Table
    The attribute table is a 64-byte region located at addresses 0x23C0-0x23FF within the
    nametable memory. Each byte corresponds to a 32x32 pixel region on the screen,
    which is further divided into 4 smaller 16x16 pixel boxes.

    Each attribute byte determines the palette ID for each of these 16x16 pixel boxes,
    allowing the PPU to assign distinct color palettes to specific screen regions.

    Attribute Byte Structure:
    7654 3210
    |||| ||++- Palette ID for the top-left 16x16 pixel box
    |||| ++--- Palette ID for the top-right 16x16 pixel box
    ||++------ Palette ID for the bottom-left 16x16 pixel box
    ++-------- Palette ID for the bottom-right 16x16 pixel box

    Example:
    Attribute Byte: 1010 0111
      |||| ||++- 11 (palette 3) for top-left box
      |||| ++--- 01 (palette 1) for top-right box
      ||++------ 10 (palette 2) for bottom-left box
      ++-------- 10 (palette 2) for bottom-right box

    Visual Representation:
      ,---- Top-left (palette 3)
      |3 1 - Top-right (palette 1)
      |2 2 - Bottom-right (palette 2)
      `---- Bottom-left (palette 2)

    Each attribute byte affects a 32x32 pixel region. The attribute table covers the
    entire screen as an 8x8 grid of 32x32 regions (totaling 256x240 pixels).
    The first 8 bytes of the attribute table correspond to the top row of 32x256 pixels,
    the next 8 bytes correspond to the next row, and so on.

    Attribute Table Coverage:
    - Top-left corner of the screen: Address 0x23C0
    - Bottom-right corner of the screen: Address 0x23FF
    */

    // The attribute is 12 bits and is composed as follows
    /*
    NN 1111 YYY XXX
    || |||| ||| +++-- high 3 bits of coarse X (x/4)
    || |||| +++------ high 3 bits of coarse Y (y/4)
    || ++++---------- attribute offset (from the 23C0 offset)
    ++--------------- nametable select
    */
    u16 const nametableSelect = ( _vramAddr.value & 0x0C00 );
    u16 const attributeOffset = 0x23C0;
    u16 const coarseY = ( _vramAddr.bit.coarseY >> 2 ) << 3;
    u16 const coarseX = ( _vramAddr.bit.coarseX >> 2 );
    u16 const attributeAddr = attributeOffset | nametableSelect | coarseY | coarseX;
    _attributeByte = Read( attributeAddr );
}

void PPU::LoadPatternPlane0Byte()
{
    /* @brief Load plane 0, the low byte of the pattern table (bg pixel data)
     */
    /*
      If the name tables provide the "which tile" and "which palette",
      the pattern table byte provides the "which pixel" and the "which palette color"

      A pattern table byte represents an 8x8 tile.
      Each bit is a 2bit color value (0-3). How can two bits fit into one bit?
      They can't. That's why two planes are used.
      Plane 0 holds the least significant bit of the color value
      Plane 1 holds the most significant bit of the color value
      A pattern table byte is grabbed in two reads, and the values are then combined

      Here's an illustration with just 4 bits
      Plane 1:   1010
      Plane 0:   1100,

      Index:     3120

      Reading vertically, 0b11 = 3, 0b01 = 1, 0b10 = 2, 0b00 = 0
      The 3, 1, 2, and 0 are the palette indeces for these pixels
     */

    // PPUCTRL dictates pattern table start as either 0KiB or 0x1000
    u16 const bgPatternOffset = _ppuCtrl.bit.patternBackground == 0 ? 0x0000 : 0x1000;

    // Tile offset, each tile is 16 bytes, so multiply by 16
    u16 const tileOffset = _nametableByte << 4;

    // Row offset, which determines the row of the tile
    u16 const rowOffset = _vramAddr.bit.fineY;

    // Fetch the pattern table byte
    _bgPlane0Byte = Read( bgPatternOffset + tileOffset + rowOffset );
}

void PPU::LoadPatternPlane1Byte()
{
    /* @brief: Load plane 1, the high byte of the pattern table (bg pixel data)
     */

    // Grab the pattern table byte for plane 1
    // Same calculation as plane 0, but add 8 to the tile offset
    u16 const bgPatternOffset = _ppuCtrl.bit.patternBackground == 0 ? 0x0000 : 0x1000;
    u16 const tileOffset = _nametableByte << 4;
    u16 const rowOffset = _vramAddr.bit.fineY;
    _bgPlane1Byte = Read( bgPatternOffset + tileOffset + rowOffset + 8 );
}

void PPU::IncrementScrollX()
{
    /* @brief: Increment the coarse X position in the VRAM address.

       @details: The coarse X position determines which tile is rendered next. It doesn't change
       the tile itself, but updates the vram address, which points to the correct nametable,
       pattern table, and attribute data (all things you need to compose a tile)

       If the coarse X value reaches the end of a nametable (tile 31), coarse X wraps back to 0
       and the nametable X bit is flipped, changing the nametable page.
     */

    // Increment coarse x, if rendering is enabled
    if ( !_isRenderingEnabled ) {
        return;
    }

    // If at end of screen, set to 0, and flip the name table x bit
    if ( _vramAddr.bit.coarseX == 31 ) {
        _vramAddr.bit.coarseX = 0;
        _vramAddr.bit.nametableX = !_vramAddr.bit.nametableX;
    } else {
        _vramAddr.bit.coarseX++;
    }
}

void PPU::IncrementScrollY()
{
    /* @brief Increments the vertical scroll Y position in the VRAM address
       @details: This function updates the fine Y and coarse Y values in the VRAM address.
       Unlike IncrementScrollX (which is updated every 8 cycles/pixels), this function executes
       once every 256 cycles (once per scanline).
     */
    // Increment scroll y if on cycle 256
    if ( _cycle != 256 ) {
        return;
    }

    // If fine y is less than 7, increment fine y
    if ( _vramAddr.bit.fineY < 7 ) {
        _vramAddr.bit.fineY++;
        return;
    }

    // Swap nametable y if coarse y is 29
    if ( _vramAddr.bit.coarseY == 29 ) {
        _vramAddr.bit.coarseY = 0;
        _vramAddr.bit.nametableY = !_vramAddr.bit.nametableY;
        return;
    }
    // Reset coarse y if land in attribute memory space
    if ( _vramAddr.bit.coarseY > 29 ) {
        _vramAddr.bit.coarseY = 0;
        return;
    }

    _vramAddr.bit.coarseY++;
}

u8 PPU::GetBgPixel() // NOLINT
{
    if ( _ppuMask.bit.renderBackground == 0 ) {
        return 0x00;
    }

    if ( _ppuMask.bit.renderBackgroundLeft == 0 && _cycle < 9 ) {
        return 0x00;
    }

    // Compute a bitmask to select the correct bit for the current pixel,
    // taking into account the fine x scrolling offset.
    u16 const mask = 0x8000 >> _fineX;
    // Extract the background pixel bits:
    // - The high pattern shifter gives a value of 2 if its bit is set.
    // - The low pattern shifter gives a value of 1 if its bit is set.
    // Combining them yields a 2-bit pixel index (0-3).
    u8 const bgPixel =
        ( ( _bgShiftPatternHigh & mask ) ? 0b10 : 0 ) | ( ( _bgShiftPatternLow & mask ) ? 0b01 : 0 );
    return bgPixel;
}

u8 PPU::GetBgPalette() // NOLINT
{
    if ( _ppuMask.bit.renderBackground == 0 ) {
        return 0x00;
    }

    if ( _ppuMask.bit.renderBackgroundLeft == 0 && _cycle < 9 ) {
        return 0x00;
    }
    // Compute a bitmask to select the correct bit for the current pixel
    u16 const mask = 0x8000 >> _fineX;
    // Combining them yields a 2-bit pixel index (0-3).
    return ( ( _bgShiftAttributeHigh & mask ) ? 0b10 : 0 ) | ( ( _bgShiftAttributeLow & mask ) ? 0b01 : 0 );
}

u8 PPU::GetSpritePalette() // NOLINT
{
    // TODO: Implement
    return 0x00;
}

u8 PPU::GetSpritePixel() // NOLINT
{

    // TODO: Implement
    return 0x00;
}

u32 PPU::GetOutputPixel( u8 bgPixel, u8 spritePixel, u8 bgPalette, u8 spritePalette ) // NOLINT
{
    // u8 fgPriority = 0; // TODO: Fetch from sprite
    // u8 pixel = 0x00;
    // u8 palette = 0x00;
    //
    // // (fg_pixel != 0) yields 1 if fg_pixel is nonzero (visible).
    // // (bg_pixel == 0) yields 1 if the background is transparent.
    // // Their bitwise combination with fg_priority tells us if we should use fg.
    // int mask = -( ( ( bgPixel != 0 ) & ( ( spritePixel == 0 ) | fgPriority ) ) );
    //
    // // If mask == -1 (all ones), the foreground values are chosen.
    // // If mask == 0, the background values are selected.
    // pixel = (uint8_t) ( ( mask & spritePixel ) | ( ( ~mask ) & bgPixel ) );
    // palette = (uint8_t) ( ( mask & spritePalette ) | ( ( ~mask ) & bgPalette ) );
    //
    // u16 paletteAddr = 0x3F00 + ( palette << 2 ) + pixel;
    // u8  paletteIdx = Read( paletteAddr ) & 0x3F;
    u8 const paletteIdx = Read( 0x3F00 ) & 0x3F;
    (void) bgPixel;
    (void) spritePixel;
    (void) bgPalette;
    (void) spritePalette;

    return _nesPaletteRgbValues[paletteIdx];
}
