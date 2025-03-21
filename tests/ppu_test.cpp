#include "bus.h"
#include "cartridge.h"
#include "config.h"
#include <fmt/base.h>
#include <gtest/gtest.h>

class PpuTest : public ::testing::Test
// This class is a test fixture that provides shared setup and teardown for all
// tests
{
  protected:
    // All tests assume flat memory model, which is why true is passed to Bus
    // constructor
    Bus        bus;
    PPU       &ppu = bus.ppu;
    CPU       &cpu = bus.cpu;
    Cartridge &cartridge = bus.cartridge;

    PpuTest()
    {
        std::string romFile = std::string( ROM_DIR ) + "/palette.nes";
        cartridge.LoadRom( romFile );
        bus.cpu.Reset();
    }
};

TEST_F( PpuTest, Write2000 )
{
    cpu.Write( 0x2000, 0x80 );
    EXPECT_EQ( ppu.ppuCtrl.bit.nmiEnable, 1 );

    ppu.ppuCtrl.bit.nametableX = 1;
    ppu.ppuCtrl.bit.nametableY = 1;
    auto data = ppu.ppuCtrl.value;
    cpu.Write( 0x2000, data );
    EXPECT_EQ( ppu.tempAddr.bit.nametableX, 1 );
    EXPECT_EQ( ppu.tempAddr.bit.nametableY, 1 );
}

TEST_F( PpuTest, Write2001 )
{
    cpu.Write( 0x2001, 0x1 );
    EXPECT_EQ( ppu.GetMaskGrayscale(), 1 );
}

TEST_F( PpuTest, Write2005Scroll ) // Scroll
{
    EXPECT_EQ( ppu.GetAddrLatch(), 0 );
    u8 data = 0x10;
    cpu.Write( 0x2005, data );
    EXPECT_EQ( ppu.GetAddrLatch(), 1 );
    EXPECT_EQ( ppu.GetTempCoarseX(), data >> 3 );
    cpu.Write( 0x2005, data );
    EXPECT_EQ( ppu.GetAddrLatch(), 0 );
    EXPECT_EQ( ppu.GetTempCoarseY(), data >> 3 );
    EXPECT_EQ( ppu.GetFineX(), data & 0x7 );
}

TEST_F( PpuTest, Write2007 )
{
    ppu.ppuCtrl.bit.vramIncrement = 0;
    cpu.Write( 0x2007, 0 );
    EXPECT_EQ( ppu.vramAddr.value, 0x1 );
    ppu.ppuCtrl.bit.vramIncrement = 1;
    cpu.Write( 0x2007, 0 );
    EXPECT_EQ( ppu.vramAddr.value, 0x1 + 32 );
}

TEST_F( PpuTest, Read2002 )
{
    // should clear vblank
    ppu.ppuStatus.bit.vBlank = 1;
    auto data = cpu.Read( 0x2002 );
    EXPECT_EQ( ppu.ppuStatus.bit.vBlank, 0 );
    EXPECT_EQ( data, 0x80 );
}

TEST_F( PpuTest, VramSetAddr )
{
    EXPECT_EQ( ppu.GetAddrLatch(), 0 );
    u8 data1 = 0x12;
    u8 data2 = 0x34;
    // First write
    cpu.Write( 0x2006, data1 );
    EXPECT_EQ( ppu.GetAddrLatch(), 1 );
    u16 tempAddr = 0x1200;
    EXPECT_EQ( ppu.GetTempAddr(), tempAddr );

    // Second write
    cpu.Write( 0x2006, data2 );
    EXPECT_EQ( ppu.GetAddrLatch(), 0 );
    u16 expectedAddr = 0x1234;
    EXPECT_EQ( ppu.GetTempAddr(), expectedAddr );
    EXPECT_EQ( ppu.vramAddr.value, expectedAddr );
}

TEST_F( PpuTest, VramReadDelay )
{
    // Write to vram address 0x2F00
    ppu.vramAddr.value = 0x2F00;
    bus.Write( 0x2007, 0x12 );
    bus.Write( 0x2007, 0x34 );

    // Vram address should have incremented by 2
    EXPECT_EQ( ppu.vramAddr.value, 0x2F02 );

    // Reading back should give the same values
    ppu.vramAddr.value = 0x2F00;
    bus.Read( 0x2007 ); // dummy read, data buffer delay
    auto data1 = bus.Read( 0x2007 );
    auto data2 = bus.Read( 0x2007 );
    EXPECT_EQ( data1, 0x12 );
    EXPECT_EQ( data2, 0x34 );
    EXPECT_EQ( ppu.vramAddr.value, 0x2F03 );
}

TEST_F( PpuTest, VramReadWrite )
{
    ppu.vramAddr.value = 0x2F00;
    bus.Write( 0x2007, 0x56 );
    ppu.vramAddr.value = 0x2F00;
    bus.Read( 0x2007 ); // delay
    auto data = bus.Read( 0x2007 );
    EXPECT_EQ( data, 0x56 );
}

TEST_F( PpuTest, VramBuffer )
{
    ppu.vramAddr.value = 0x2F00;
    bus.Write( 0x2007, 0x78 );
    ppu.vramAddr.value = 0x2F00;
    bus.Read( 0x2007 ); // dummy read
    // write a new value (0x12). This should not affect the buffered value.
    bus.Write( 0x2007, 0x12 );
    // read the buffered value. It should still be 0x78.
    auto bufferedValue = bus.Read( 0x2007 );
    EXPECT_EQ( bufferedValue, 0x78 );
}

TEST_F( PpuTest, VramBufferPaletteIsolation )
{
    ppu.vramAddr.value = 0x2F00;
    bus.Write( 0x2007, 0x9A );
    ppu.vramAddr.value = 0x2F00;
    bus.Read( 0x2007 ); // dummy read

    // write to a palette location (0x3F00), which should not affect the read
    // buffer.
    ppu.vramAddr.value = 0x3F00;
    bus.Write( 0x2007, 0x34 );

    // change back to a non-palette address to ensure buffer reads are re-enabled.
    ppu.vramAddr.value = 0x2F00;

    // read the buffered value. It should still be 0x9A.
    auto bufferedValue = bus.Read( 0x2007 );
    EXPECT_EQ( bufferedValue, 0x9A );
}

TEST_F( PpuTest, PaletteWriteRead )
{
    // Set palette entry at index 0 to 0x12.
    ppu.vramAddr.value = 0x3F00;
    bus.Write( 0x2007, 0x12 );
    ppu.vramAddr.value = 0x2F00;
    bus.Read( 0x2007 ); // fill buffer with dummy value
    ppu.vramAddr.value = 0x3F00;
    auto data = bus.Read( 0x2007 ); // should directly read the palette instead of the buffer
    EXPECT_EQ( data, 0x12 );
}

TEST_F( PpuTest, PaletteMirroring )
{
    // Write 0x12 to palette entry at index 0.
    ppu.vramAddr.value = 0x3F00;
    bus.Write( 0x2007, 0x12 );
    // Write 0x34 to palette entry at index 0xE0.
    // Since palette addresses are masked to 0x1F, index 0xE0 & 0x1F == 0x00.
    ppu.vramAddr.value = 0x3FE0;
    bus.Write( 0x2007, 0x34 );
    // Now read palette entry at index 0.
    auto data = ppu.GetPaletteEntry( 0x00 );
    // Expect that the mirror reflects the value from index 0xE0.
    EXPECT_EQ( data, 0x34 );
}
TEST_F( PpuTest, PaletteMirrorFrom10To00 )
{
    // Write 0x12 to palette entry at index 0.
    ppu.vramAddr.value = 0x3F00;
    bus.Write( 0x2007, 0x12 );
    // Write 0x34 to palette entry at index 0x10.
    // By masking, 0x10 maps to index 0x00.
    ppu.vramAddr.value = 0x3F10;
    bus.Write( 0x2007, 0x34 );
    auto data = ppu.GetPaletteEntry( 0x00 );
    EXPECT_EQ( data, 0x34 );
}

TEST_F( PpuTest, PaletteMirrorFrom00To10 )
{
    ppu.vramAddr.value = 0x3F10;
    bus.Write( 0x2007, 0x12 );
    ppu.vramAddr.value = 0x3F00;
    bus.Write( 0x2007, 0x34 );
    auto data = ppu.GetPaletteEntry( 0x10 );
    EXPECT_EQ( data, 0x34 );
}

TEST_F( PpuTest, SpriteRamReadWrite )
{
    bus.Write( 0x2003, 0x00 );
    bus.Write( 0x2004, 0x12 );
    bus.Write( 0x2003, 0x00 );
    u8 data = bus.Read( 0x2004 );
    EXPECT_EQ( data, 0x12 );
}

TEST_F( PpuTest, SpriteWriteIncrement )
{
    bus.Write( 0x2003, 0x00 );
    bus.Write( 0x2004, 0x12 );
    bus.Write( 0x2004, 0x34 );
    bus.Write( 0x2003, 0x01 );
    u8 data = bus.Read( 0x2004 );
    EXPECT_EQ( data, 0x34 );
}

TEST_F( PpuTest, SpriteReadNoIncrement )
{
    bus.Write( 0x2003, 0x00 );
    bus.Write( 0x2004, 0x12 );
    bus.Write( 0x2004, 0x34 );
    bus.Write( 0x2003, 0x00 );
    u8 firstRead = bus.Read( 0x2004 );
    u8 secondRead = bus.Read( 0x2004 );
    EXPECT_EQ( firstRead, secondRead );
}

TEST_F( PpuTest, SpriteThirdByteMasked )
{
    bus.Write( 0x2003, 0x03 );
    bus.Write( 0x2004, 0xFF );
    bus.Write( 0x2003, 0x03 );
    u8 data = bus.Read( 0x2004 );
    EXPECT_EQ( data, 0xE3 );
}

TEST_F( PpuTest, SpriteDmaBasic )
{

    static const u8 testData[4] = { 0x12, 0x82, 0xE3, 0x78 };
    bus.Write( 0x200, 0x12 );
    bus.Write( 0x201, 0x82 );
    bus.Write( 0x202, 0xE3 );
    bus.Write( 0x203, 0x78 );

    // Set OAM address to 0.
    bus.Write( 0x2003, 0x00 );
    bus.Write( 0x4014, 0x02 );
    while ( bus.dmaInProgress ) {
        bus.Clock();
    }
    for ( int i = 0; i < 4; i++ ) {
        EXPECT_EQ( ppu.oam.data[i], testData[i] );
    }
}

TEST_F( PpuTest, SpriteDmaCopyWrap )
{
    static const u8 testData[4] = { 0x12, 0x82, 0xE3, 0x78 };

    // Write testData into CPU memory starting at address 0x200.
    bus.Write( 0x200, testData[0] );
    bus.Write( 0x201, testData[1] );
    bus.Write( 0x202, testData[2] );
    bus.Write( 0x203, testData[3] );

    // Set OAM address near the end, e.g. 0xFE.
    bus.Write( 0x2003, 0xFE );

    // Trigger DMA copy by writing 0x02 to $4014 (DMA source = 0x200).
    bus.Write( 0x4014, 0x02 );

    // Process DMA until it's finished.
    while ( bus.dmaInProgress ) {
        bus.Clock();
    }

    // Expected behavior with initial OAMADDR = 0xFE:
    // dmaOffset 0 -> destination = (0xFE + 0) mod 256 = 0xFE should receive
    // testData[0] dmaOffset 1 -> destination = (0xFE + 1) mod 256 = 0xFF should
    // receive testData[1] dmaOffset 2 -> destination = (0xFE + 2) mod 256 = 0x00
    // should receive testData[2] dmaOffset 3 -> destination = (0xFE + 3) mod 256
    // = 0x01 should receive testData[3]
    EXPECT_EQ( ppu.oam.data[0xFE], testData[0] );
    EXPECT_EQ( ppu.oam.data[0xFF], testData[1] );
    EXPECT_EQ( ppu.oam.data[0x00], testData[2] );
    EXPECT_EQ( ppu.oam.data[0x01], testData[3] );
}

TEST_F( PpuTest, SpriteDmaCopyPreservesOamAddr )
{
    // Set OAM address to 1.
    bus.Write( 0x2003, 0x01 );
    // Trigger DMA copy.
    bus.Write( 0x4014, 0x02 );
    while ( bus.dmaInProgress ) {
        bus.ProcessDma();
    }
    // Now write 0xFF to OAMDATA.
    bus.Write( 0x2004, 0xFF );
    // Reset OAM address to 1.
    bus.Write( 0x2003, 0x01 );
    u8 data = bus.Read( 0x2004 );
    EXPECT_EQ( data, 0xFF );
}

TEST_F( PpuTest, ResolveNameTableAddress_SingleUpper )
{
    u16 addr = 0x23AB;
    // Expected: 0x2000 OR (addr & 0x03FF)
    u16 expected = 0x2000 | ( addr & 0x03FF );
    u16 result = ppu.ResolveNameTableAddress( addr, static_cast<int>( MirrorMode::SingleUpper ) );
    EXPECT_EQ( result, expected );
}

TEST_F( PpuTest, ResolveNameTableAddress_SingleLower )
{
    u16 addr = 0x2434;
    u16 expected = 0x2800 | ( addr & 0x03FF );
    u16 result = ppu.ResolveNameTableAddress( addr, static_cast<int>( MirrorMode::SingleLower ) );
    EXPECT_EQ( result, expected );
}

TEST_F( PpuTest, ResolveNameTableAddress_Vertical )
{
    // Test an address in NT0 range.
    u16 addr1 = 0x2000;
    u16 expected1 = 0x2000 | ( addr1 & 0x07FF );
    u16 result1 = ppu.ResolveNameTableAddress( addr1, static_cast<int>( MirrorMode::Vertical ) );
    EXPECT_EQ( result1, expected1 );

    // Test an address in the mirrored portion.
    u16 addr2 = 0x2C00;
    // 0x2C00 & 0x07FF gives 0x0400, so expected address is 0x2000 + 0x0400 =
    // 0x2400.
    u16 expected2 = 0x2000 | ( addr2 & 0x07FF );
    u16 result2 = ppu.ResolveNameTableAddress( addr2, static_cast<int>( MirrorMode::Vertical ) );
    EXPECT_EQ( result2, expected2 );
}

TEST_F( PpuTest, ResolveNameTableAddress_Horizontal )
{
    // Address in NT0 region (both 0x2000-0x23FF and its mirror 0x2400-0x27FF
    // should resolve to NT0).
    u16 addr1 = 0x2400;                          // should map to 0x2000 + (addr1 mod 0x400)
    u16 expected1 = 0x2000 | ( addr1 & 0x03FF ); // 0x2400 & 0x03FF is 0, so becomes 0x2000.
    u16 result1 = ppu.ResolveNameTableAddress( addr1, static_cast<int>( MirrorMode::Horizontal ) );
    EXPECT_EQ( result1, expected1 );

    // Address in NT1 region (both 0x2800-0x2BFF and its mirror 0x2C00-0x2FFF
    // should resolve to NT1).
    u16 addr2 = 0x2C00;                          // should map to 0x2800 + (addr2 mod 0x400)
    u16 expected2 = 0x2800 | ( addr2 & 0x03FF ); // For 0x2C00, (addr2 & 0x03FF) is 0.
    u16 result2 = ppu.ResolveNameTableAddress( addr2, static_cast<int>( MirrorMode::Horizontal ) );
    EXPECT_EQ( result2, expected2 );
}

TEST_F( PpuTest, ResolveNameTableAddress_FourScreen )
{
    u16 addr = 0x2FFF;
    u16 expected = addr & 0x0FFF;
    u16 result = ppu.ResolveNameTableAddress( addr, static_cast<int>( MirrorMode::FourScreen ) );
    EXPECT_EQ( result, expected );
}

TEST_F( PpuTest, ClearSecondaryOam )
{
    cpu.Write( 0x2001, 0x08 ); // enable rendering

    // set secondaryOam to non-default
    for ( u8 i = 0; i < 32; i++ ) {
        ppu.secondaryOam.data.at( i ) = 0x12;
    }
    for ( u8 i = 0; i < 32; i++ ) {
        EXPECT_EQ( ppu.secondaryOam.data.at( i ), 0x12 );
    }

    ppu.cycle = 1;
    for ( int i = 0; i < 64; i++ ) {
        ppu.Tick();
    }

    // All should be intialized to 0xFF.
    for ( u8 i = 0; i < 32; i++ ) {
        EXPECT_EQ( ppu.secondaryOam.data.at( i ), 0xFF );
    }
}

TEST_F( PpuTest, EvalSpriteStart )
{
    cpu.Write( 0x2001, 0x08 ); // enable rendering

    // Pre-set non-default values to ensure they get reset.
    ppu.sprite0Added = true;
    ppu.spriteInRange = true;
    ppu.secondaryOamAddr = 10;
    ppu.oamCopyDone = true;
    ppu.oamAddr = 0x04;
    ppu.spriteAddrHi = 0x20;
    ppu.spriteAddrLo = 0x02;
    ppu.scanline = 100;
    ppu.cycle = 65;
    ppu.Tick();

    // Now, verify that SpriteEvalForNextScanlineStart() has reinitialized the
    // evaluation state.
    EXPECT_FALSE( ppu.sprite0Added ) << "sprite0Added should be reset to false";
    EXPECT_FALSE( ppu.spriteInRange ) << "spriteInRange should be reset to false";
    EXPECT_EQ( ppu.secondaryOamAddr, 0 ) << "secondaryOamAddr should be reset to 0";
    EXPECT_FALSE( ppu.oamCopyDone ) << "oamCopyDone should be false";

    // Verify that spriteAddrHi and spriteAddrLo are computed from oamAddr.
    u8 expectedHi = ( ppu.oamAddr >> 2 ) & 0x3F;
    u8 expectedLo = ppu.oamAddr & 0x03;
    EXPECT_EQ( ppu.spriteAddrHi, expectedHi ) << "spriteAddrHi should be (oamAddr >> 2) & 0x3F";
    EXPECT_EQ( ppu.spriteAddrLo, expectedLo ) << "spriteAddrLo should be oamAddr & 0x03";

    // Verify that firstVisibleSpriteAddr and lastVisibleSpriteAddr are set to
    // spriteAddrHi * 4.
    EXPECT_EQ( ppu.firstVisibleSpriteAddr, expectedHi * 4 )
        << "firstVisibleSpriteAddr should equal spriteAddrHi * 4";
    EXPECT_EQ( ppu.lastVisibleSpriteAddr, expectedHi * 4 )
        << "lastVisibleSpriteAddr should equal spriteAddrHi * 4";
}

TEST_F( PpuTest, EvalSprites_Sprite0Added )
{
    cpu.Write( 0x2001, 0x08 ); // enable rendering

    // Set cycle to 66 (even cycle right after cycle 65).
    auto resetTest = [this]() {
        ppu.sprite0Added = false;
        ppu.spriteInRange = false;
        ppu.oamAddr = 0;
        ppu.spriteAddrHi = 0;
        ppu.spriteAddrLo = 0;
        ppu.secondaryOamAddr = 0;
        ppu.oamCopyDone = false;
        ppu.scanline = 100;
        ppu.cycle = 66;
        for ( int i = 0; i < 32; i++ ) {
            ppu.secondaryOam.data.at( i ) = 0x12;
        }
    };

    // Simulate that the sprite's Y value is such that the sprite is in range for
    // the nexet scanline. scanline >= y && scanline + 1 < y +
    // (ppuCtrl.bit.spriteSize ? 16 : 8) So for scanline 100, an 8 pixel sprite
    // should be in range if its y values are 94-101.
    for ( int yVal = 94; yVal < 102; yVal++ ) {
        resetTest();
        ppu.oamCopyBuffer = yVal;
        ppu.SpriteEvalForNextScanline();
        EXPECT_TRUE( ppu.sprite0Added ) << "Sprite 0 flag should be set on cycle 66 if in range.";
    }
    // Out of range, 93 and 102, should fail
    resetTest();
    ppu.oamCopyBuffer = 93;
    ppu.SpriteEvalForNextScanline();
    EXPECT_FALSE( ppu.sprite0Added ) << "Sprite 0 flag should not be set on cycle 66 if out of range.";
    resetTest();
    ppu.oamCopyBuffer = 102;
    ppu.SpriteEvalForNextScanline();
    EXPECT_FALSE( ppu.sprite0Added ) << "Sprite 0 flag should not be set on cycle 66 if out of range.";

    // When ppuCtrl.bit.spriteSize is set, the range should account for 16 pixel
    // tall sprites so the valid y range is now 86-101.
    for ( int yVal = 86; yVal < 102; yVal++ ) {
        resetTest();
        ppu.ppuCtrl.bit.spriteSize = 1;
        ppu.oamCopyBuffer = yVal;
        ppu.SpriteEvalForNextScanline();
        EXPECT_TRUE( ppu.sprite0Added ) << "Sprite 0 flag range fail for 16 pixel sprites. yVal: " << yVal;
    }

    // Out of range: 85 and 102 should fail
    resetTest();
    ppu.ppuCtrl.bit.spriteSize = 1;
    ppu.oamCopyBuffer = 85;
    ppu.SpriteEvalForNextScanline();
    EXPECT_FALSE( ppu.sprite0Added ) << "Sprite 0 flag should not be set on cycle 66 if out of range.";
    resetTest();
    ppu.ppuCtrl.bit.spriteSize = 1;
    ppu.oamCopyBuffer = 102;
    ppu.SpriteEvalForNextScanline();
    EXPECT_FALSE( ppu.sprite0Added ) << "Sprite 0 flag should not be set on cycle 66 if out of range.";
}

TEST_F( PpuTest, EvalSprites_OddCycle )
{
    // Set up environment for visible scanline.
    cpu.Write( 0x2001, 0x08 ); // Enable rendering
    ppu.scanline = 100;
    ppu.cycle = 67; // odd cycle in the evaluation window (e.g., 67)

    // Set primary OAM with known data.
    // For example, assume oam.data[0x00] = 0x55.
    ppu.oam.data[0] = 0x55;
    // Set oamAddr to 0.
    ppu.oamAddr = 0x00;

    // Call SpriteEvalForNextScanline().
    ppu.SpriteEvalForNextScanline();

    // On odd cycle, oamCopyBuffer should equal the value read from primary OAM.
    EXPECT_EQ( ppu.oamCopyBuffer, 0x55 );

    // Also, oamAddr should be updated based on spriteAddrLo and spriteAddrHi.
    u8 expectedOamAddr = ( ppu.spriteAddrLo & 0x03 ) | ( ppu.spriteAddrHi << 2 );
    EXPECT_EQ( ppu.oamAddr, expectedOamAddr );
}

TEST_F( PpuTest, EvalSprites_SecondaryOamFill )
{
    cpu.Write( 0x2001, 0x08 ); // Enable rendering
    ppu.scanline = 100;        // Arbitrary visible scanline

    // Set 8 sprites (4 bytes each)
    // in range (Y values: 94-101 for scanline+1 = 101)
    for ( int i = 0; i < 8; i++ ) {
        ppu.oam.entries.at( i ) = {
            .y = (u8) ( 94 + i ),    // in range for scanline+1 = 101
            .tileIndex = 0x10,       // arbitrary tile id
            .attribute = 0x00,       // no special attributes
            .x = (u8) ( 10 + i * 5 ) // spread out horizontally
        };
    }

    // initialize secondaryOam
    while ( ppu.cycle < 65 ) {
        ppu.Tick();
    }

    // All the sprites should be in range, and since they're copied every other
    // cycle, we should Tick for (8 * 4) * 2 = 64 cycles.
    for ( int i = 0; i < 64; i++ ) {
        ppu.Tick();
    }

    // Ensure that the copied sprites match the first 8 in OAM
    for ( int i = 0; i < 8; i++ ) {
        EXPECT_EQ( ppu.secondaryOam.entries.at( i ).y, ppu.oam.entries.at( i ).y );
        EXPECT_EQ( ppu.secondaryOam.entries.at( i ).tileIndex, ppu.oam.entries.at( i ).tileIndex );
        EXPECT_EQ( ppu.secondaryOam.entries.at( i ).attribute, ppu.oam.entries.at( i ).attribute );
        EXPECT_EQ( ppu.secondaryOam.entries.at( i ).x, ppu.oam.entries.at( i ).x );
    }

    // Verify that sprite overflow is NOT set (since exactly 8 were copied)
    EXPECT_EQ( ppu.ppuStatus.bit.spriteOverflow, 0 )
        << "Sprite overflow should not be set with exactly 8 sprites.";
}

TEST_F( PpuTest, EvalSprites_SecondaryOamOverflow )
{
    cpu.Write( 0x2001, 0x08 );
    ppu.scanline = 100;

    // Set 9 sprites (36 bytes)
    for ( int i = 0; i < 9; i++ ) {
        ppu.oam.entries.at( i ) = {
            .y = static_cast<u8>( 94 + i ),        // In range for scanline+1 = 101
            .tileIndex = 0x10,                     // Arbitrary tile ID
            .attribute = 0x00,                     // No special attributes
            .x = static_cast<u8>( 10 + ( i * 5 ) ) // Spread out horizontally
        };
    }

    // initialize secondaryOam
    while ( ppu.cycle < 65 ) {
        ppu.Tick();
    }

    // Copy all 9 sprites
    // ( 9 * 4 ) * 2 = 72 cycles
    for ( int i = 0; i < 72; i++ ) {
        ppu.Tick();
    }

    // Ensure the first 8 sprites match the OAM entries
    for ( int i = 0; i < 8; i++ ) {
        EXPECT_EQ( ppu.secondaryOam.entries.at( i ).y, ppu.oam.entries.at( i ).y );
        EXPECT_EQ( ppu.secondaryOam.entries.at( i ).tileIndex, ppu.oam.entries.at( i ).tileIndex );
        EXPECT_EQ( ppu.secondaryOam.entries.at( i ).attribute, ppu.oam.entries.at( i ).attribute );
        EXPECT_EQ( ppu.secondaryOam.entries.at( i ).x, ppu.oam.entries.at( i ).x );
    }

    // Verify that sprite overflow is set
    EXPECT_EQ( ppu.ppuStatus.bit.spriteOverflow, 1 )
        << "Sprite overflow flag should be set when more than 8 sprites are "
           "found.";
}

TEST_F( PpuTest, FetchNametableByte )
{
    ppu.vramAddr.value = 0x23AB;
    u16 testAddr = 0x2000 | ( ppu.vramAddr.value & 0x0FFF );
    ppu.WriteVram( testAddr, 0x12 );
    EXPECT_EQ( ppu.nametableByte, 0x00 );
    ppu.FetchNametableByte();
    EXPECT_EQ( ppu.nametableByte, 0x12 );
}

TEST_F( PpuTest, FetchAttributeByte )
{
    ppu.WriteVram( 0x23C0, 0xFF );
    ppu.WriteVram( 0x23FF, 0xAA ); // 10101010

    // Test (0,0) coarse coords (should fetch from 0x23C0)
    ppu.vramAddr.bit.coarseX = 0;
    ppu.vramAddr.bit.coarseY = 0;
    ppu.FetchAttributeByte();
    EXPECT_EQ( ppu.attributeByte, 0x03 ) << "Attribute byte should be 0x03 (lower 2 bits of 0xFF)";

    // Test (31, 29) coarse coords (should fetch from 0x23FF)
    ppu.vramAddr.bit.coarseX = 31;
    ppu.vramAddr.bit.coarseY = 29;
    ppu.FetchAttributeByte();
    EXPECT_EQ( ppu.attributeByte, 0x02 ) << "Attribute byte should be 0x02 (bits from 0xAA)";
}

TEST_F( PpuTest, FetchBgPatternBytes )
{
    u16 targetAddr = 0x0000;
    ppu.ppuCtrl.bit.patternBackground = 0; // 0x0000
    ppu.nametableByte = 0x00;              // tile 0
    ppu.vramAddr.bit.fineY = 0;            // row offset 0
    cartridge.SetChrROM( targetAddr, 0x55 );
    cartridge.SetChrROM( targetAddr + 8, 0xAA );

    ppu.FetchBgPattern0Byte();
    EXPECT_EQ( ppu.bgPattern0Byte, 0x55 ) << "Pattern low byte should be 0x55";
    ppu.FetchBgPattern1Byte();
    EXPECT_EQ( ppu.bgPattern1Byte, 0xAA ) << "Pattern high byte should be 0xAA";

    ppu.ppuCtrl.bit.patternBackground = 1; // 0x1000
    ppu.nametableByte = 0x10;              // tile 16
    ppu.vramAddr.bit.fineY = 1;            // row offset 1
    targetAddr = 0x1000 + ( 0x10 << 4 ) + 1;
    cartridge.SetChrROM( targetAddr, 0x12 );
    cartridge.SetChrROM( targetAddr + 8, 0x34 );

    ppu.FetchBgPattern0Byte();
    EXPECT_EQ( ppu.bgPattern0Byte, 0x12 ) << "Pattern low byte should be 0x12";
    ppu.FetchBgPattern1Byte();
    EXPECT_EQ( ppu.bgPattern1Byte, 0x34 ) << "Pattern high byte should be 0x34";
}

TEST_F( PpuTest, FetchBgTileData_TopLeft )
{
    // Integration test for the above Fetches..
    // Cycles 1-256 and 321-336, every 8 ppu cycles:
    // fetch nametable byte, attribute byte, pattern0, pattern1, load the bg
    // shifters, and increment coarseX.

    // enable bg rendering
    bus.Write( 0x2001, 0x08 );

    // Fetch top left tile data
    ppu.vramAddr.value = 0x0000;
    u16 nametableAddr = 0x2000;
    u8  tileIdxData = 0x24;
    ppu.WriteVram( nametableAddr, tileIdxData );

    u16 attributeAddr = 0x23C0;
    u8  attibuteData = 0xAA;
    ppu.WriteVram( attributeAddr, attibuteData );

    ppu.ppuCtrl.bit.patternBackground = 1;
    u16 patternTableAddr = 0x1240;
    u8  pattern0Data = 0x12;
    u8  pattern1Data = 0x34;
    cartridge.SetChrROM( patternTableAddr, pattern0Data );
    cartridge.SetChrROM( patternTableAddr + 8, pattern1Data );

    ppu.cycle = 1;
    for ( int i = 0; i < 8; i++ ) {
        ppu.Tick();
    }
    EXPECT_EQ( ppu.nametableByte, tileIdxData );
    EXPECT_EQ( ppu.attributeByte, 0x02 );
    EXPECT_EQ( ppu.bgPattern0Byte, pattern0Data );
    EXPECT_EQ( ppu.bgPattern1Byte, pattern1Data );
    EXPECT_EQ( ppu.bgPatternShiftLow, pattern0Data );
    EXPECT_EQ( ppu.bgPatternShiftHigh, pattern1Data );
    EXPECT_EQ( ppu.bgAttributeShiftLow, 0x00 );
    EXPECT_EQ( ppu.bgAttributeShiftHigh, 0xFF );
    EXPECT_EQ( ppu.vramAddr.bit.coarseX, 1 );
}

TEST_F( PpuTest, FetchBgTileData_Arbitrary )
{
    // Sampled from an arbitrary value using Mesen.

    bus.Write( 0x2001, 0x08 ); // enable bg rendering
    ppu.ppuCtrl.bit.patternBackground = 1;
    cartridge.SetMirrorMode( MirrorMode::Vertical );

    ppu.vramAddr.value = 0x0C0A;
    // nametable byte
    ppu.WriteVram( 0x2C0A, 0x24 );
    // attribute byte (acquired using Mesen)
    ppu.WriteVram( 0x2FC2, 0x00 );

    u16 patternTableAddr = 0x1000 + ( 0x24 << 4 ) + ppu.vramAddr.bit.fineY;
    u8  pattern0Data = 0x12;
    u8  pattern1Data = 0x34;
    cartridge.SetChrROM( patternTableAddr, pattern0Data );
    cartridge.SetChrROM( patternTableAddr + 8, pattern1Data );

    ppu.cycle = 1;
    for ( int i = 0; i < 8; i++ ) {
        ppu.Tick();
    }
    EXPECT_EQ( ppu.nametableByte, 0x24 );
    EXPECT_EQ( ppu.attributeByte, 0x00 );
    EXPECT_EQ( ppu.bgPattern0Byte, pattern0Data );
    EXPECT_EQ( ppu.bgPattern1Byte, pattern1Data );
    EXPECT_EQ( ppu.bgPatternShiftLow, pattern0Data );
    EXPECT_EQ( ppu.bgPatternShiftHigh, pattern1Data );
    EXPECT_EQ( ppu.bgAttributeShiftLow, 0x00 );
    EXPECT_EQ( ppu.bgAttributeShiftHigh, 0x00 );
    EXPECT_EQ( ppu.vramAddr.value, 0x0C0B );
}

TEST_F( PpuTest, LoadBgShifters )
{
    ppu.bgPatternShiftLow = 0xFF00;
    ppu.bgPatternShiftHigh = 0xAA00;
    ppu.bgAttributeShiftLow = 0xFF12;
    ppu.bgAttributeShiftHigh = 0xAA34;
    ppu.bgPattern0Byte = 0x55;
    ppu.bgPattern1Byte = 0x33;

    // Attribute byte = 0b11 (both low and high mask)
    ppu.attributeByte = 0x03;

    ppu.LoadBgShifters();

    EXPECT_EQ( ppu.bgPatternShiftLow, 0xFF55 );
    EXPECT_EQ( ppu.bgPatternShiftHigh, 0xAA33 );
    EXPECT_EQ( ppu.bgAttributeShiftLow, 0xFFFF );
    EXPECT_EQ( ppu.bgAttributeShiftHigh, 0xAAFF );
}

TEST_F( PpuTest, IncrementCoarseX )
{
    cpu.Write( 0x2001, 0x08 ); // Rendering enabled

    // Test simple increment (0 → 1)
    ppu.vramAddr.bit.coarseX = 0;
    ppu.vramAddr.bit.nametableX = 0;
    ppu.IncrementCoarseX();
    EXPECT_EQ( ppu.vramAddr.bit.coarseX, 1 );
    EXPECT_EQ( ppu.vramAddr.bit.nametableX, 0 );

    // Test wrap-around (31 → 0, nametableX flips)
    ppu.vramAddr.bit.coarseX = 31;
    ppu.vramAddr.bit.nametableX = 0;
    ppu.IncrementCoarseX();
    EXPECT_EQ( ppu.vramAddr.bit.coarseX, 0 );
    EXPECT_EQ( ppu.vramAddr.bit.nametableX, 1 );

    // And back again (31 → 0, nametableX flips again)
    ppu.vramAddr.bit.coarseX = 31;
    ppu.vramAddr.bit.nametableX = 1;
    ppu.IncrementCoarseX();
    EXPECT_EQ( ppu.vramAddr.bit.coarseX, 0 );
    EXPECT_EQ( ppu.vramAddr.bit.nametableX, 0 );
}

TEST_F( PpuTest, IncrementCoarseY )
{
    cpu.Write( 0x2001, 0x08 ); // Rendering enabled

    // Fine Y < 7, should just increment fine Y
    ppu.vramAddr.bit.fineY = 6;
    ppu.IncrementCoarseY();
    EXPECT_EQ( ppu.vramAddr.bit.fineY, 7 );

    // Incrementing again should reset fine Y and increment coarse Y
    ppu.IncrementCoarseY();
    EXPECT_EQ( ppu.vramAddr.bit.fineY, 0 );
    EXPECT_EQ( ppu.vramAddr.bit.coarseY, 1 );

    // Coarse Y = 29, should flip nametable Y when fine y is 7 and increment
    // Non-attribute memory
    ppu.vramAddr.bit.coarseY = 29;
    ppu.vramAddr.bit.fineY = 7;
    ppu.vramAddr.bit.nametableY = 0;
    ppu.IncrementCoarseY();
    EXPECT_EQ( ppu.vramAddr.bit.coarseY, 0 );
    EXPECT_EQ( ppu.vramAddr.bit.fineY, 0 );
    EXPECT_EQ( ppu.vramAddr.bit.nametableY, 1 );

    // When in attribute memory, coarse Y should wrap back to zero when incremented from 31
    ppu.vramAddr.bit.coarseY = 31;
    ppu.vramAddr.bit.fineY = 7;
    ppu.vramAddr.bit.nametableY = 1;
    ppu.IncrementCoarseY();
    EXPECT_EQ( ppu.vramAddr.bit.coarseY, 0 );
    EXPECT_EQ( ppu.vramAddr.bit.fineY, 0 );
    EXPECT_EQ( ppu.vramAddr.bit.nametableY, 1 );
}

TEST_F( PpuTest, ShiftBgRegisters )
{
    // Write to enable mask render background
    ppu.ppuMask.bit.renderBackground = 1;

    // Initialize registers with known values
    ppu.bgPatternShiftLow = 0b0000000011111111;
    ppu.bgPatternShiftHigh = 0b1111111100000000;
    ppu.bgAttributeShiftLow = 0b0000000010101010;
    ppu.bgAttributeShiftHigh = 0b1111111101010101;

    ppu.ShiftBgRegisters();

    // Verify each register has been shifted left by one bit
    EXPECT_EQ( ppu.bgPatternShiftLow, 0b0000000111111110 );
    EXPECT_EQ( ppu.bgPatternShiftHigh, 0b1111111000000000 );
    EXPECT_EQ( ppu.bgAttributeShiftLow, 0b0000000101010100 );
    EXPECT_EQ( ppu.bgAttributeShiftHigh, 0b1111111010101010 );
}

TEST_F( PpuTest, ShiftSpriteRegisters )
{
    ppu.scanline = 120;
    ppu.cycle = 128;
    ppu.ppuMask.bit.renderSprites = 1;

    // Initialize shift registers with a known value for easy verification
    for ( int i = 0; i < 8; ++i ) {
        ppu.spriteShiftLow.at( i ) = 0b00000001;
        ppu.spriteShiftHigh.at( i ) = 0b10000000;
    }

    // Verify behavior for each sprite
    // Sprites with XCounter = 0 should shift left
    ppu.ShiftSpriteRegisters();
    EXPECT_EQ( ppu.spriteShiftLow.at( 0 ), 0b00000010 );
    EXPECT_EQ( ppu.spriteShiftHigh.at( 0 ), 0b00000000 );
    EXPECT_EQ( ppu.spriteShiftLow.at( 3 ), 0b00000010 );
    EXPECT_EQ( ppu.spriteShiftHigh.at( 3 ), 0b00000000 );
    EXPECT_EQ( ppu.spriteShiftLow.at( 4 ), 0b00000010 );
    EXPECT_EQ( ppu.spriteShiftHigh.at( 4 ), 0b00000000 );
    EXPECT_EQ( ppu.spriteShiftLow.at( 6 ), 0b00000010 );
    EXPECT_EQ( ppu.spriteShiftHigh.at( 6 ), 0b00000000 );

    // Sprites with XCounter > 0 should decrement their counter without shifting
    EXPECT_EQ( ppu.spriteShiftLow.at( 1 ), 0b00000001 );  // unchanged
    EXPECT_EQ( ppu.spriteShiftHigh.at( 1 ), 0b10000000 ); // unchanged

    EXPECT_EQ( ppu.spriteShiftLow.at( 2 ), 0b00000001 );  // unchanged
    EXPECT_EQ( ppu.spriteShiftHigh.at( 2 ), 0b10000000 ); // unchanged

    EXPECT_EQ( ppu.spriteShiftLow.at( 5 ), 0b00000001 );  // unchanged
    EXPECT_EQ( ppu.spriteShiftHigh.at( 5 ), 0b10000000 ); // unchanged

    EXPECT_EQ( ppu.spriteShiftLow.at( 7 ), 0b00000001 );  // unchanged
    EXPECT_EQ( ppu.spriteShiftHigh.at( 7 ), 0b10000000 ); // unchanged
}

TEST_F( PpuTest, ShiftSpriteRegisters_NoShiftOutsideVisibleRange )
{
    // Scanline and cycle outside visible rendering range
    ppu.scanline = 240;
    ppu.cycle = 300;

    for ( int i = 0; i < 8; ++i ) {
        ppu.spriteShiftLow.at( i ) = 0x01;
        ppu.spriteShiftHigh.at( i ) = 0x01;
    }

    ppu.ShiftSpriteRegisters();

    // Expect no shifts to occur
    for ( int i = 0; i < 8; ++i ) {
        EXPECT_EQ( ppu.spriteShiftLow.at( i ), 0x01 );
        EXPECT_EQ( ppu.spriteShiftHigh.at( i ), 0x01 );
    }
}

TEST_F( PpuTest, OddFrameSkip )
{
    ppu.Reset();

    // even, 340 ticks lands on cycle 340
    ppu.frame = 2;
    ppu.ppuMask.bit.renderBackground = 1;
    ppu.cycle = 0;
    for ( int i = 0; i < 340; i++ ) {
        ppu.Tick();
    }
    EXPECT_EQ( ppu.cycle, 340 );

    // odd, 340 ticks lands on cycle 0
    ppu.frame = 3;
    ppu.cycle = 0;
    for ( int i = 0; i < 340; i++ ) {
        ppu.Tick();
    }
    EXPECT_EQ( ppu.cycle, 0 );
}

TEST_F( PpuTest, VBlankPeriod )
{
    cpu.Write( 0x2001, 0x08 );

    // Enter VBlank
    ppu.scanline = 241;
    ppu.cycle = 1;
    ppu.Tick();
    EXPECT_EQ( ppu.ppuStatus.bit.vBlank, 1 );

    // Exit VBlank
    ppu.scanline = 261;
    ppu.cycle = 1;
    ppu.Tick();
    EXPECT_EQ( ppu.ppuStatus.bit.vBlank, 0 );
}

TEST_F( PpuTest, TransferAddressX )
{
    bus.Write( 0x2001, 0x08 ); // enable rendering
    ppu.cycle = 257;

    ppu.tempAddr.bit.nametableX = 1;
    ppu.tempAddr.bit.coarseX = 1;
    ppu.TransferAddressX();
    EXPECT_EQ( ppu.vramAddr.bit.nametableX, 1 );
    EXPECT_EQ( ppu.vramAddr.bit.coarseX, 1 );
}

TEST_F( PpuTest, UpdateFrameBuffer_IsBufferFull )
{
    // Should not be full
    ppu.Reset();
    bool &isFull = ppu.isBufferFull;
    EXPECT_FALSE( isFull );

    // One entry before full
    for ( int i = 0; i < 61439; i++ ) {
        ppu.UpdateFrameBuffer( 0x00 );
    }
    EXPECT_FALSE( isFull );
    ppu.UpdateFrameBuffer( 0x00 );
    EXPECT_TRUE( isFull );
}

TEST_F( PpuTest, PrerenderScanline )
{
    // This function should:
    // 1. clear status registers
    // 2. clear the sprite shift registers
    // 3. Transfer Y address from temp to vram

    // enable rendering
    bus.Write( 0x2001, 0x08 );

    // Setup state
    ppu.ppuStatus.value = 0xFF;
    for ( int i = 0; i < 8; i++ ) {
        ppu.spriteShiftLow.at( i ) = 0xFF;
        ppu.spriteShiftHigh.at( i ) = 0xFF;
    }
    ppu.tempAddr.bit.fineY = 1;
    ppu.tempAddr.bit.nametableY = 1;
    ppu.tempAddr.bit.coarseY = 1;

    // When not called on 261, should do nothing
    ppu.scanline = 260;
    ppu.PrerenderScanline();
    EXPECT_EQ( ppu.ppuStatus.value, 0xFF );
    for ( int i = 0; i < 8; i++ ) {
        EXPECT_EQ( ppu.spriteShiftLow.at( i ), 0xFF );
        EXPECT_EQ( ppu.spriteShiftHigh.at( i ), 0xFF );
    }
    EXPECT_EQ( ppu.tempAddr.bit.fineY, 1 );
    EXPECT_EQ( ppu.tempAddr.bit.nametableY, 1 );
    EXPECT_EQ( ppu.tempAddr.bit.coarseY, 1 );

    // Called on 261
    ppu.scanline = 261;

    // cycle 1 clears status registers and sprite shift registers
    ppu.cycle = 1;
    ppu.PrerenderScanline();
    EXPECT_EQ( ppu.ppuStatus.bit.vBlank, 0 );
    EXPECT_EQ( ppu.ppuStatus.bit.spriteOverflow, 0 );
    EXPECT_EQ( ppu.ppuStatus.bit.spriteZeroHit, 0 );
    for ( int i = 0; i < 8; i++ ) {
        EXPECT_EQ( ppu.spriteShiftLow.at( i ), 0x00 );
        EXPECT_EQ( ppu.spriteShiftHigh.at( i ), 0x00 );
    }

    // cycle 280 - 304 traansfers Y address from temp to vram
    ppu.cycle = 280;
    ppu.PrerenderScanline();
    EXPECT_EQ( ppu.vramAddr.bit.fineY, 1 );
    EXPECT_EQ( ppu.vramAddr.bit.nametableY, 1 );
    EXPECT_EQ( ppu.vramAddr.bit.coarseY, 1 );
}

TEST_F( PpuTest, GetBackgroundPixel )
{
    // Set cycle to a valid drawing cycle (e.g., 50) and fineX = 0.
    ppu.cycle = 50;
    ppu.fineX = 0;

    // Enable background rendering and unmask left-side background.
    ppu.ppuMask.bit.renderBackground = 1;
    ppu.ppuMask.bit.renderBackgroundLeft = 1;
    // Disable sprites to avoid sprite interference.
    ppu.ppuMask.bit.renderSprites = 0;

    // Set up background pattern shift registers.
    // With fineX = 0, bitMask = 0x8000.
    // To get a 2 (binary 10) background pixel, we want:
    // - Low plane: bit15 = 0.
    // - High plane: bit15 = 1.
    ppu.bgPatternShiftLow = 0x0000;
    ppu.bgPatternShiftHigh = 0x8000;

    // Set up background attribute shifters.
    // To get an attribute value of 1 (binary 01), we want:
    // - Low attribute shift: bit15 = 1.
    // - High attribute shift: bit15 = 0.
    ppu.bgAttributeShiftLow = 0x8000;
    ppu.bgAttributeShiftHigh = 0x0000;

    // Ensure that our background is enabled in our emulator flag if needed.
    // (In our GetOutputPixel() code, if background is enabled, backgroundColor becomes spriteBgColor.)
    // For this test, we assume that if ppuMask.bit.renderBackground is true, background is drawn.

    // Since sprites are disabled, no sprite pixels are processed.
    ppu.spriteCount = 0;

    // With the above settings, the background extraction yields:
    //   p0 = 0, p1 = 1, so bgPixel = (1 << 1) | 0 = 2.
    //   a0 = 1, a1 = 0, so bgPalette = (0 << 1) | 1 = 1.
    // Then finalPixel = bgPixel = 2 and finalPalette = bgPalette = 1.
    // The final palette address is computed as:
    //   0x3F00 + (1 << 2) + 2 = 0x3F00 + 4 + 2 = 0x3F06.

    // Write a known value to VRAM at address 0x3F06.
    ppu.WriteVram( 0x3F06, 0x10 );
    // Set the master palette entry for index 0x10 to a known color.
    ppu.nesPaletteRgbValues[0x10] = 0xAABBCCDD;

    // Call GetOutputPixel() and expect it to return 0xAABBCCDD.
    u32 pixelColor = ppu.GetOutputPixel();
    EXPECT_EQ( pixelColor, 0xAABBCCDD );
}

TEST_F( PpuTest, GetOutputPixel_LeftSideBackgroundMasking )
{
    // Set cycle in the left 8 pixels (e.g., cycle = 5)
    ppu.cycle = 5;
    ppu.fineX = 0; // fine scroll 0
    // Enable background rendering but mask left 8 pixels.
    ppu.ppuMask.bit.renderBackground = 1;
    ppu.ppuMask.bit.renderBackgroundLeft = 0;
    // Disable sprite rendering for isolation.
    ppu.ppuMask.bit.renderSprites = 0;

    // Force background shift registers to some nonzero values,
    // but they should not be used because cycle < 9 and left is masked.
    ppu.bgPatternShiftLow = 0x8000;
    ppu.bgPatternShiftHigh = 0x0000;
    ppu.bgAttributeShiftLow = 0x0000;
    ppu.bgAttributeShiftHigh = 0x0000;

    // Since cycle < 9 and left mask is off, no background pixel is drawn:
    // bgPixel remains 0, and thus finalPixel and finalPalette remain 0.

    // Write a known palette value into VRAM at 0x3F00.
    // (Final palette address = 0x3F00 + (0<<2) + 0 = 0x3F00)
    ppu.WriteVram( 0x3F00, 0x12 );
    // Set our master palette so that index 0x12 maps to a known color.
    ppu.nesPaletteRgbValues.at( 0x12 ) = 0x11223344;

    // Call GetOutputPixel() and verify that it returns our expected color.
    u32 outColor = ppu.GetOutputPixel();
    EXPECT_EQ( outColor, 0x11223344 );
}

TEST_F( PpuTest, GetOutputPixel_LeftSideSpriteMasking )
{
    // Set cycle in left region.
    ppu.cycle = 5;
    ppu.fineX = 0;
    // Disable background rendering to isolate sprite layer.
    ppu.ppuMask.bit.renderBackground = 0;
    // Enable sprite rendering but mask left side.
    ppu.ppuMask.bit.renderSprites = 1;
    ppu.ppuMask.bit.renderSpritesLeft = 0;

    // Even if we set up a sprite in secondary OAM,
    // the sprite processing block won't run when cycle < 9.
    ppu.spriteCount = 1;
    // Initialize sprite shift registers arbitrarily.
    ppu.spriteShiftLow[0] = 0x80; // would yield pixel 1 if processed.
    ppu.spriteShiftHigh[0] = 0x00;
    // Also set sprite X value in secondary OAM so that sprite would normally be visible.
    ppu.secondaryOam.entries[0] = {
        .y = 50, .tileIndex = 0x10, .attribute = 0x00, .x = 0 }; // X = 0 means active.

    // Since cycle is less than 9 for sprites, no sprite pixel is fetched.
    // Also, background is disabled.
    // Final pixel will then fall to the fallback which is 0.
    // Write known value to VRAM at 0x3F00.
    ppu.WriteVram( 0x3F00, 0x34 );
    ppu.nesPaletteRgbValues.at( 0x34 ) = 0x55667788;

    u32 outColor = ppu.GetOutputPixel();
    EXPECT_EQ( outColor, 0x55667788 );
}

TEST_F( PpuTest, GetOutputPixel_SpriteZeroHitAndPriority )
{
    // Use a drawing cycle in normal range.
    ppu.cycle = 50;
    ppu.fineX = 0;
    // Enable both background and sprite rendering.
    ppu.ppuMask.bit.renderBackground = 1;
    ppu.ppuMask.bit.renderBackgroundLeft = 1;
    ppu.ppuMask.bit.renderSprites = 1;
    ppu.ppuMask.bit.renderSpritesLeft = 1;

    // Set up background shift registers to produce a background pixel.
    // Let background pixel be 1 (p0=1, p1=0) and attribute 0, so bgPalette = 0.
    ppu.bgPatternShiftLow = 0x8000;  // bit15 set => p0 = 1.
    ppu.bgPatternShiftHigh = 0x0000; // p1 = 0.
    ppu.bgAttributeShiftLow = 0x0000;
    ppu.bgAttributeShiftHigh = 0x0000;

    // Set up sprite evaluation.
    // Simulate that sprite 0 is active and its X counter is 0.
    ppu.spriteCount = 1;
    // Set sprite 0's shift registers so that the pixel equals 2 (binary 10).
    // For a non-flipped sprite: bit7 of high plane must be 1 and bit7 of low plane 0.
    ppu.spriteShiftLow[0] = 0x00;
    ppu.spriteShiftHigh[0] = 0x80;
    // Mark sprite zero as added.
    ppu.sprite0Added = true;
    // Configure sprite 0 attributes in secondary OAM:
    // - Lower two bits = 1, so palette offset = 1 + 4 = 5.
    // - Bit 5 clear indicates high priority.
    ppu.secondaryOam.entries[0] = { .y = 50, .tileIndex = 0x10, .attribute = 0x01, .x = 0 };

    // In the sprite loop inside GetOutputPixel:
    // - Sprite 0 will produce spritePixel = 2.
    // - spritePalette = (0x01 & 0x03) + 0x04 = 5.
    // Meanwhile, background produces bgPixel = 1 and bgPalette = 0.
    // With spritePriority = 1 (since attribute bit 5 is clear),
    // the sprite pixel should take precedence.
    // Also, since sprite0Added and spriteZeroRendered are true,
    // a sprite-zero hit should be registered.

    // Write a known palette index to VRAM.
    // Final palette address calculation:
    //   finalPalette = 5 and finalPixel = 2, so address = 0x3F00 + (5 << 2) + 2 = 0x3F00 + 20 + 2 = 0x3F16.
    // Write a valid index (0x0A, which is within 0-63) at 0x3F16.
    ppu.WriteVram( 0x3F16, 0x0A );
    // Set the corresponding master palette entry to a known color.
    ppu.nesPaletteRgbValues.at( 0x0A ) = 0xDEADBEEF;

    // Clear sprite zero hit flag before calling.
    ppu.ppuStatus.bit.spriteZeroHit = 0;

    u32 outColor = ppu.GetOutputPixel();

    // Verify that the returned color matches the expected sprite output.
    EXPECT_EQ( outColor, 0xDEADBEEF );
    // Verify that the sprite zero hit flag was set.
    EXPECT_EQ( ppu.ppuStatus.bit.spriteZeroHit, 1 );
}

int main( int argc, char **argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
