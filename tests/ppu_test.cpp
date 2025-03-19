// Google Test Basics: TEST and TEST_F
//
// - `TEST`: Defines a standalone test case.
// - `TEST_F`: Defines a test case that uses a fixture (a shared setup/teardown
// environment).
//
// Common Assertions:
// - EXPECT_EQ(val1, val2): Checks if val1 == val2 (non-fatal, continues test on
// failure).
// - ASSERT_EQ(val1, val2): Checks if val1 == val2 (fatal, stops test on
// failure).
// - EXPECT_NE, EXPECT_LT, EXPECT_GT, EXPECT_LE, EXPECT_GE: Comparison macros.
// - EXPECT_TRUE(condition) / EXPECT_FALSE(condition): Checks a boolean
// condition.
//
// Basic Usage:
// 1. Include <gtest/gtest.h>.
// 2. Define tests using `TEST` or `TEST_F`.
// 3. Compile using settings from CMakeLists.txt and run with `ctest`.
//
// Below is an example of both `TEST` and `TEST_F` usage.

#include "bus.h"
#include "cartridge.h"
#include <fmt/base.h>
#include <gtest/gtest.h>
#include "config.h"

class PpuTest : public ::testing::Test
// This class is a test fixture that provides shared setup and teardown for all tests
{
  protected:
    // All tests assume flat memory model, which is why true is passed to Bus constructor
    Bus  bus;
    PPU &ppu = bus.ppu;
    CPU &cpu = bus.cpu;

    PpuTest()
    {
        std::string romFile = std::string( ROM_DIR ) + "/palette.nes";
        bus.cartridge.LoadRom( romFile );
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
    ppu.ppuStatus.bit.verticalBlank = 1;
    auto data = cpu.Read( 0x2002 );
    EXPECT_EQ( ppu.ppuStatus.bit.verticalBlank, 0 );
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

    // write to a palette location (0x3F00), which should not affect the read buffer.
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
    // dmaOffset 0 -> destination = (0xFE + 0) mod 256 = 0xFE should receive testData[0]
    // dmaOffset 1 -> destination = (0xFE + 1) mod 256 = 0xFF should receive testData[1]
    // dmaOffset 2 -> destination = (0xFE + 2) mod 256 = 0x00 should receive testData[2]
    // dmaOffset 3 -> destination = (0xFE + 3) mod 256 = 0x01 should receive testData[3]
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
    // 0x2C00 & 0x07FF gives 0x0400, so expected address is 0x2000 + 0x0400 = 0x2400.
    u16 expected2 = 0x2000 | ( addr2 & 0x07FF );
    u16 result2 = ppu.ResolveNameTableAddress( addr2, static_cast<int>( MirrorMode::Vertical ) );
    EXPECT_EQ( result2, expected2 );
}

TEST_F( PpuTest, ResolveNameTableAddress_Horizontal )
{
    // Address in NT0 region (both 0x2000-0x23FF and its mirror 0x2400-0x27FF should resolve to NT0).
    u16 addr1 = 0x2400;                          // should map to 0x2000 + (addr1 mod 0x400)
    u16 expected1 = 0x2000 | ( addr1 & 0x03FF ); // 0x2400 & 0x03FF is 0, so becomes 0x2000.
    u16 result1 = ppu.ResolveNameTableAddress( addr1, static_cast<int>( MirrorMode::Horizontal ) );
    EXPECT_EQ( result1, expected1 );

    // Address in NT1 region (both 0x2800-0x2BFF and its mirror 0x2C00-0x2FFF should resolve to NT1).
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

    // Now, verify that EvaluateSpritesStart() has reinitialized the evaluation state.
    EXPECT_FALSE( ppu.sprite0Added ) << "sprite0Added should be reset to false";
    EXPECT_FALSE( ppu.spriteInRange ) << "spriteInRange should be reset to false";
    EXPECT_EQ( ppu.secondaryOamAddr, 0 ) << "secondaryOamAddr should be reset to 0";
    EXPECT_FALSE( ppu.oamCopyDone ) << "oamCopyDone should be false";

    // Verify that spriteAddrHi and spriteAddrLo are computed from oamAddr.
    u8 expectedHi = ( ppu.oamAddr >> 2 ) & 0x3F;
    u8 expectedLo = ppu.oamAddr & 0x03;
    EXPECT_EQ( ppu.spriteAddrHi, expectedHi ) << "spriteAddrHi should be (oamAddr >> 2) & 0x3F";
    EXPECT_EQ( ppu.spriteAddrLo, expectedLo ) << "spriteAddrLo should be oamAddr & 0x03";

    // Verify that firstVisibleSpriteAddr and lastVisibleSpriteAddr are set to spriteAddrHi * 4.
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

    // Simulate that the sprite's Y value is such that the sprite is in range for the nexet scanline.
    // scanline >= y && scanline + 1 < y + (ppuCtrl.bit.spriteSize ? 16 : 8)
    // So for scanline 100, an 8 pixel sprite should be in range if its y values are 94-101.
    for ( int yVal = 94; yVal < 102; yVal++ ) {
        resetTest();
        ppu.oamCopyBuffer = yVal;
        ppu.EvaluateSprites();
        EXPECT_TRUE( ppu.sprite0Added ) << "Sprite 0 flag should be set on cycle 66 if in range.";
    }
    // Out of range, 93 and 102, should fail
    resetTest();
    ppu.oamCopyBuffer = 93;
    ppu.EvaluateSprites();
    EXPECT_FALSE( ppu.sprite0Added ) << "Sprite 0 flag should not be set on cycle 66 if out of range.";
    resetTest();
    ppu.oamCopyBuffer = 102;
    ppu.EvaluateSprites();
    EXPECT_FALSE( ppu.sprite0Added ) << "Sprite 0 flag should not be set on cycle 66 if out of range.";

    // When ppuCtrl.bit.spriteSize is set, the range should account for 16 pixel tall sprites
    // so the valid y range is now 86-101.
    for ( int yVal = 86; yVal < 102; yVal++ ) {
        resetTest();
        ppu.ppuCtrl.bit.spriteSize = 1;
        ppu.oamCopyBuffer = yVal;
        ppu.EvaluateSprites();
        EXPECT_TRUE( ppu.sprite0Added ) << "Sprite 0 flag range fail for 16 pixel sprites. yVal: " << yVal;
    }

    // Out of range: 85 and 102 should fail
    resetTest();
    ppu.ppuCtrl.bit.spriteSize = 1;
    ppu.oamCopyBuffer = 85;
    ppu.EvaluateSprites();
    EXPECT_FALSE( ppu.sprite0Added ) << "Sprite 0 flag should not be set on cycle 66 if out of range.";
    resetTest();
    ppu.ppuCtrl.bit.spriteSize = 1;
    ppu.oamCopyBuffer = 102;
    ppu.EvaluateSprites();
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

    // Call EvaluateSprites().
    ppu.EvaluateSprites();

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
            .id = 0x10,              // arbitrary tile id
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
        EXPECT_EQ( ppu.secondaryOam.entries.at( i ).id, ppu.oam.entries.at( i ).id );
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
            .y = static_cast<u8>( 94 + i ),    // In range for scanline+1 = 101
            .id = 0x10,                        // Arbitrary tile ID
            .attribute = 0x00,                 // No special attributes
            .x = static_cast<u8>( 10 + i * 5 ) // Spread out horizontally
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
        EXPECT_EQ( ppu.secondaryOam.entries.at( i ).id, ppu.oam.entries.at( i ).id );
        EXPECT_EQ( ppu.secondaryOam.entries.at( i ).attribute, ppu.oam.entries.at( i ).attribute );
        EXPECT_EQ( ppu.secondaryOam.entries.at( i ).x, ppu.oam.entries.at( i ).x );
    }

    // Verify that sprite overflow is set
    EXPECT_EQ( ppu.ppuStatus.bit.spriteOverflow, 1 )
        << "Sprite overflow flag should be set when more than 8 sprites are found.";
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

TEST_F( PpuTest, FetchPatternBytes )
{
    u16 targetAddr = 0x0000;
    ppu.ppuCtrl.bit.patternBackground = 0; // 0x0000
    ppu.nametableByte = 0x00;              // tile 0
    ppu.vramAddr.bit.fineY = 0;            // row offset 0
    bus.cartridge.SetChrROM( targetAddr, 0x55 );
    bus.cartridge.SetChrROM( targetAddr + 8, 0xAA );

    ppu.FetchPattern0Byte();
    EXPECT_EQ( ppu.bgPatternShiftLow, 0x55 ) << "Pattern low byte should be 0x55";
    ppu.FetchPattern1Byte();
    EXPECT_EQ( ppu.bgPatternShiftHigh, 0xAA ) << "Pattern high byte should be 0xAA";

    ppu.ppuCtrl.bit.patternBackground = 1; // 0x1000
    ppu.nametableByte = 0x10;              // tile 16
    ppu.vramAddr.bit.fineY = 1;            // row offset 1
    targetAddr = 0x1000 + ( 0x10 << 4 ) + 1;
    bus.cartridge.SetChrROM( targetAddr, 0x12 );
    bus.cartridge.SetChrROM( targetAddr + 8, 0x34 );

    ppu.FetchPattern0Byte();
    EXPECT_EQ( ppu.bgPatternShiftLow, 0x12 ) << "Pattern low byte should be 0x12";
    ppu.FetchPattern1Byte();
    EXPECT_EQ( ppu.bgPatternShiftHigh, 0x34 ) << "Pattern high byte should be 0x34";
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

TEST_F( PpuTest, ShiftBgRegisters )
{
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
    ppu.spriteXCounters = { 0, 1, 2, 0, 0, 5, 0, 3 };

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
    EXPECT_EQ( ppu.spriteXCounters.at( 1 ), 0 );          // was 1, should now be 0
    EXPECT_EQ( ppu.spriteShiftLow.at( 1 ), 0b00000001 );  // unchanged
    EXPECT_EQ( ppu.spriteShiftHigh.at( 1 ), 0b10000000 ); // unchanged

    EXPECT_EQ( ppu.spriteXCounters.at( 2 ), 1 );          // was 2, now 1
    EXPECT_EQ( ppu.spriteShiftLow.at( 2 ), 0b00000001 );  // unchanged
    EXPECT_EQ( ppu.spriteShiftHigh.at( 2 ), 0b10000000 ); // unchanged

    EXPECT_EQ( ppu.spriteXCounters.at( 5 ), 4 );          // was 5, now 4
    EXPECT_EQ( ppu.spriteShiftLow.at( 5 ), 0b00000001 );  // unchanged
    EXPECT_EQ( ppu.spriteShiftHigh.at( 5 ), 0b10000000 ); // unchanged

    EXPECT_EQ( ppu.spriteXCounters.at( 7 ), 2 );          // was 3, now 2
    EXPECT_EQ( ppu.spriteShiftLow.at( 7 ), 0b00000001 );  // unchanged
    EXPECT_EQ( ppu.spriteShiftHigh.at( 7 ), 0b10000000 ); // unchanged
}

TEST_F( PpuTest, ShiftSpriteRegisters_NoShiftOutsideVisibleRange )
{
    // Scanline and cycle outside visible rendering range
    ppu.scanline = 240;
    ppu.cycle = 300;

    ppu.spriteXCounters = { 0, 0, 0, 0, 0, 0, 0, 0 };
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

TEST_F( PpuTest, RenderPixel_Sprite0HitDetection )
{
    // TODO: Implement
}

int main( int argc, char **argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
