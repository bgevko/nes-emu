// NOLINTBEGIN(cppcoreguidelines-owning-memory, cppcoreguidelines-avoid-non-const-global-variables,
// modernize-use-trailing-return-type)
#include <gtest/gtest.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "cpu.h"
#include "json.hpp"

using json = nlohmann::json;
bool verbose = false;

class CPUTest : public ::testing::Test
{
   protected:
    CPU cpu;  // NOLINT
    // Helper methods
    void        LoadStateFromJson( const json& jsonData, const std::string& state );
    void        PrintCPUState( const json& jsonData, const std::string& state );
    void        RunTestCase( const json& testCase );
    static void PrintTestStartMsg( const std::string& testName );
    static void PrintTestEndMsg( const std::string& testName );
    static auto ExtractTestsFromJson( const std::string& path ) -> json;
    static auto ParseStatus( u8 status ) -> std::string;
};

TEST_F( CPUTest, IMM )
{
    std::string testName = "Immediate Addressing Mode IMM";
    PrintTestStartMsg( testName );
    cpu.SetPC( 0x8000 );
    EXPECT_EQ( cpu.GetPC(), 0x8000 );
    u16 addr = cpu.IMM();
    EXPECT_EQ( addr, 0x8000 );
    EXPECT_EQ( cpu.GetPC(), 0x8001 );
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, ZPG )
{
    std::string testName = "Zero Page Addressing Mode ZPG";
    PrintTestStartMsg( testName );
    cpu.SetPC( 0x0000 );
    cpu.Write( 0x0000, 0x42 );
    u16 addr = cpu.ZPG();
    EXPECT_EQ( addr, 0x42 );
    EXPECT_EQ( cpu.GetPC(), 0x0001 );
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, ZPGX )
{
    std::string testName = "Zero Page X Addressing Mode ZPGX";
    PrintTestStartMsg( testName );
    cpu.SetPC( 0x0000 );
    cpu.SetX( 0x1 );

    // Write the zero page address at the pc location
    cpu.Write( cpu.GetPC(), 0x80 );  // Zero page address 0x80
    u8 zeroPageAddr = cpu.Read( cpu.GetPC() );
    u8 expectedAddr = ( zeroPageAddr + cpu.GetX() ) & 0xFF;

    // Write a test value at the expected address
    cpu.Write( expectedAddr, 0x42 );

    u16 addr = cpu.ZPGX();
    EXPECT_EQ( addr, expectedAddr )
        << "Expected " << int( expectedAddr ) << ", but got " << int( addr );
    EXPECT_EQ( cpu.Read( addr ), 0x42 ) << "Expected 0x42, but got " << int( cpu.Read( addr ) );
    EXPECT_EQ( cpu.GetPC(), 0x0001 );
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, ZPGY )
{
    std::string testName = "Zero Page Y Addressing Mode ZPGY";
    PrintTestStartMsg( testName );
    cpu.SetPC( 0x0000 );
    cpu.SetY( 0x1 );

    // Write the zero page address at the pc location
    cpu.Write( cpu.GetPC(), 0x80 );  // Zero page address 0x80
    u8 zeroPageAddr = cpu.Read( cpu.GetPC() );
    u8 expectedAddr = ( zeroPageAddr + cpu.GetY() ) & 0xFF;

    // Write a test value at the expected address
    cpu.Write( expectedAddr, 0x42 );

    u16 addr = cpu.ZPGY();
    EXPECT_EQ( addr, expectedAddr )
        << "Expected " << int( expectedAddr ) << ", but got " << int( addr );
    EXPECT_EQ( cpu.Read( addr ), 0x42 ) << "Expected 0x42, but got " << int( cpu.Read( addr ) );
    EXPECT_EQ( cpu.GetPC(), 0x0001 );
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, ABS )
{
    std::string testName = "Absolute Addressing Mode ABS";
    PrintTestStartMsg( testName );
    cpu.SetPC( 0x0000 );
    cpu.Write( 0x0000, 0x42 );
    cpu.Write( 0x0001, 0x24 );
    u16 addr = cpu.ABS();
    EXPECT_EQ( addr, 0x2442 );
    EXPECT_EQ( cpu.GetPC(), 0x0002 );
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, IND )
{
    std::string testName = "Indirect Addressing Mode IND";
    PrintTestStartMsg( testName );
    cpu.SetPC( 0x0000 );

    // Write the low and high byte of the pointer to the pc location
    cpu.Write( 0x0000, 0xCD );
    cpu.Write( 0x0001, 0xAB );

    // Write the effective address to the pointer location
    u16 ptr = 0xABCD;
    cpu.Write( ptr, 0x34 );
    cpu.Write( ptr + 1, 0x12 );

    // Get the effective address
    u16 addr = cpu.IND();

    // Ensure the address is 0xABCD
    EXPECT_EQ( addr, 0x1234 ) << "Expected 0x1234, but got " << std::hex << addr;

    // Write a value at the effective address
    cpu.Write( addr, 0xEF );
    EXPECT_EQ( cpu.Read( addr ), 0xEF ) << "Expected 0xEF, but got " << int( cpu.Read( addr ) );

    // Ensure the pc is incremented by 2
    EXPECT_EQ( cpu.GetPC(), 0x0002 );

    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, IND_Bug )
{
    std::string testName = "Indirect Addressing Mode Bug IND";
    PrintTestStartMsg( testName );
    cpu.SetPC( 0x0000 );

    cpu.Write( 0x0000, 0xFF );
    cpu.Write( 0x0001, 0x02 );
    u16 ptr = 0x02FF;

    // Write the low byte of the effective address to the pointer location
    cpu.Write( ptr, 0x34 );
    // Place where the high byte will be read from due to bug
    cpu.Write( 0x0200, 0x12 );
    // Place where the high byte will not be read from, but would have been without the bug
    cpu.Write( 0x0300, 0x56 );

    u16 bugAddr = 0x1234;
    cpu.Write( bugAddr, 0xEF );

    u16 noBugAddr = 0x5634;
    cpu.Write( noBugAddr, 0xAB );

    u16 effectiveAddr = cpu.IND();
    EXPECT_EQ( effectiveAddr, bugAddr ) << "Expected 0x1234, but got " << std::hex << effectiveAddr;
    EXPECT_EQ( cpu.Read( effectiveAddr ), 0xEF )
        << "Expected 0xEF, but got " << int( cpu.Read( effectiveAddr ) );
    EXPECT_EQ( cpu.GetPC(), 0x0002 );

    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, INDX )
{
    std::string testName = "Indirect X Addressing Mode INDX";
    PrintTestStartMsg( testName );
    cpu.SetPC( 0x0000 );
    cpu.SetX( 0x10 );

    // Write the operand at the pc location
    u8 operand = 0x23;
    cpu.Write( 0x0000, operand );
    u8  ptr = ( operand + cpu.GetX() ) & 0xFF;
    u16 effectiveAddr = 0xABCD;

    // Write the effective address to zero-page memory
    cpu.Write( ptr, effectiveAddr & 0xFF );               // Low byte
    cpu.Write( ( ptr + 1 ) & 0xFF, effectiveAddr >> 8 );  // High byte

    // Write a test value at the effective address
    cpu.Write( effectiveAddr, 0x42 );

    u16 addr = cpu.INDX();
    EXPECT_EQ( addr, effectiveAddr )
        << "Expected " << std::hex << effectiveAddr << ", but got " << addr;
    EXPECT_EQ( cpu.Read( addr ), 0x42 ) << "Expected 0x42, but got " << int( cpu.Read( addr ) );
    EXPECT_EQ( cpu.GetPC(), 0x0001 );

    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, INDY )
{
    std::string testName = "Indirect Y Addressing Mode INDY";
    PrintTestStartMsg( testName );

    cpu.SetPC( 0x0000 );
    cpu.SetY( 0x10 );

    u8 operand = 0x23;
    cpu.Write( 0x0000, operand );
    u8  ptr = operand;
    u16 effectiveAddr = 0xAB00;

    // Write the effective address to zero page memory
    cpu.Write( ptr, effectiveAddr & 0xFF );    // Low byte
    cpu.Write( ptr + 1, effectiveAddr >> 8 );  // High byte

    u16 finalAddr = effectiveAddr + cpu.GetY();
    cpu.Write( finalAddr, 0x42 );  // Write a test value at the final address
    u16 addr = cpu.INDY();
    EXPECT_EQ( addr, finalAddr ) << "Expected " << std::hex << finalAddr << ", but got " << addr;
    EXPECT_EQ( cpu.Read( addr ), 0x42 ) << "Expected 0x42, but got " << int( cpu.Read( addr ) );
    EXPECT_EQ( cpu.GetPC(), 0x0001 );

    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, REL )
{
    std::string testName = "Relative Addressing Mode REL";
    PrintTestStartMsg( testName );
    cpu.SetPC( 0x1000 );

    // Write a relative address of -5 at the pc location
    cpu.Write( cpu.GetPC(), 0xFB );
    u16 backBranch = cpu.REL();
    EXPECT_EQ( backBranch, 0x0FFB ) << "Expected 0x0FFB, but got " << std::hex << backBranch;
    EXPECT_EQ( cpu.GetPC(), 0x1001 );

    // Write a relative address of +5 at the pc location
    cpu.SetPC( 0x1000 );
    cpu.Write( cpu.GetPC(), 0x05 );
    u16 forwardBranch = cpu.REL();
    EXPECT_EQ( forwardBranch, 0x1005 ) << "Expected 0x1005, but got " << std::hex << forwardBranch;
    EXPECT_EQ( cpu.GetPC(), 0x1001 );
}

TEST_F( CPUTest, x00_Break )
{
    std::string testName = "00 BRK";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/a9.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, x81_STA_IndirectX )
{
    std::string testName = "81 STA IndirectX";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/81.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, x84_STY_ZeroPage )
{
    std::string testName = "84 STY ZeroPage";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/84.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, x85_STA_ZeroPage )
{
    std::string testName = "85 STA ZeroPage";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/85.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, x86_STX_ZeroPage )
{
    std::string testName = "86 STX ZeroPage";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/86.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, x8C_STY_Absolute )
{
    std::string testName = "8C STY Absolute";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/8c.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, x8D_STA_Absolute )
{
    std::string testName = "8D STA Absolute";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/8d.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, x8E_STX_Absolute )
{
    std::string testName = "8E STX Absolute";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/8e.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, x91_STA_IndirectY )
{
    std::string testName = "91 STA IndirectY";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/91.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, x94_STY_ZeroPageX )
{
    std::string testName = "94 STY ZeroPageX";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/94.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, x95_STA_ZeroPageX )
{
    std::string testName = "95 STA ZeroPageX";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/95.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, x96_STX_ZeroPageY )
{
    std::string testName = "96 STX ZeroPageY";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/96.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, x99_STA_AbsoluteY )
{
    std::string testName = "99 STA AbsoluteY";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/99.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, x9D_STA_AbsoluteX )
{
    std::string testName = "9D STA AbsoluteX";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/9d.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xA0_LDY_Immediate )
{
    std::string testName = "A0 LDY Immediate";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/a0.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xA1_LDA_Xindirect )
{
    std::string testName = "A1 LDA X-Indirect";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/a1.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xA2_LDX_Immediate )
{
    std::string testName = "A2 LDX Immediate";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/a2.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xA4_LDY_ZeroPage )
{
    std::string testName = "A4 LDY ZeroPage";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/a4.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xA5_LDA_ZeroPage )
{
    std::string testName = "A5 LDA ZeroPage";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/a5.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xA6_LDX_ZeroPage )
{
    std::string testName = "A6 LDX ZeroPage";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/a6.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xAC_LDY_Absolute )
{
    std::string testName = "AC LDY Absolute";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/ac.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xAD_LDA_Absolute )
{
    std::string testName = "AD LDA Absolute";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/ad.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xAE_LDX_Absolute )
{
    std::string testName = "AE LDX Absolute";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/ae.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xA9_LDA_Immediate )
{
    std::string testName = "A9 LDA Immediate";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/a9.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xB1_LDA_IndirectY )
{
    std::string testName = "B1 LDA IndirectY";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/b1.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xB4_LDY_ZeroPageX )
{
    std::string testName = "B4 LDY ZeroPageX";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/b4.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xB5_LDA_ZeroPageX )
{
    std::string testName = "B5 LDA ZeroPageX";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/b5.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xB6_LDX_ZeroPageY )
{
    std::string testName = "B6 LDX ZeroPageY";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/b6.json" );

    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xB9_LDA_AbsoluteY )
{
    std::string testName = "B9 LDA AbsoluteY";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/b9.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xBC_LDY_AbsoluteX )
{
    std::string testName = "BC LDY AbsoluteX";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/bc.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xBD_LDA_AbsoluteX )
{
    std::string testName = "BD LDA AbsoluteX";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/bd.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

TEST_F( CPUTest, xBE_LDX_AbsoluteY )
{
    std::string testName = "BE LDX AbsoluteY";
    PrintTestStartMsg( testName );
    json testCases = ExtractTestsFromJson( "tests/HARTE/be.json" );
    for ( const auto& testCase : testCases )
    {
        RunTestCase( testCase );
    }
    PrintTestEndMsg( testName );
}

int main( int argc, char** argv )
{
    for ( int i = 1; i < argc; ++i )
    {
        if ( std::string( argv[i] ) == "--v" )
        {
            verbose = true;
            std::cout << "Verbose mode enabled.\n";
            break;
        }
    }
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------
// ----------------------- Helper Functions -----------------------
// ----------------------------------------------------------------

void CPUTest::LoadStateFromJson( const json& jsonData, const std::string& state )
{
    cpu.SetPC( u16( jsonData[state]["pc"] ) );
    cpu.SetA( jsonData[state]["a"] );
    cpu.SetX( jsonData[state]["x"] );
    cpu.SetY( jsonData[state]["y"] );
    cpu.SetS( jsonData[state]["s"] );
    cpu.SetP( jsonData[state]["p"] );
    for ( const auto& ramEntry : jsonData[state]["ram"] )
    {
        uint16_t address = ramEntry[0];
        uint8_t  value = ramEntry[1];
        cpu.Write( address, value );
    }
}

// Helper function to print CPU state
void CPUTest::PrintCPUState( const json& jsonData, const std::string& state )
{
    // Expected values
    u16 expectedPC = u16( jsonData[state]["pc"] );
    u8  expectedA = jsonData[state]["a"];
    u8  expectedX = jsonData[state]["x"];
    u8  expectedY = jsonData[state]["y"];
    u8  expectedS = jsonData[state]["s"];
    u8  expectedP = jsonData[state]["p"];

    // Actual values
    u16 actualPC = cpu.GetPC();
    u8  actualA = cpu.GetA();
    u8  actualX = cpu.GetX();
    u8  actualY = cpu.GetY();
    u8  actualS = cpu.GetS();
    u8  actualP = cpu.GetP();

    // Column Widths
    const int labelWidth = 6;
    const int valueWidth = 12;

    // Print header
    std::cout << "----------" << state << " State----------" << '\n';
    std::cout << std::left << std::setw( labelWidth ) << "" << std::setw( valueWidth ) << "EXPECTED"
              << std::setw( valueWidth ) << "ACTUAL" << '\n';

    // Function to format and print a line
    auto printLine =
        [&]( const std::string& label, const std::string& expected, const std::string& actual )
    {
        std::cout << std::left << std::setw( labelWidth ) << label;
        std::cout << std::setw( valueWidth ) << expected;
        std::cout << std::setw( valueWidth ) << actual << '\n';
    };

    // Convert values to strings with proper formatting
    std::ostringstream oss;
    oss << std::hex << std::setw( 4 ) << std::setfill( '0' ) << expectedPC;
    std::string expectedPCStr = oss.str();
    oss.str( "" );
    oss.clear();
    oss << std::hex << std::setw( 4 ) << std::setfill( '0' ) << actualPC;
    std::string actualPCStr = oss.str();
    oss.str( "" );
    oss.clear();

    // Reset formatting
    std::cout << std::dec << std::setfill( ' ' );

    // Print registers
    printLine( "pc:", expectedPCStr, actualPCStr );
    printLine( "s:", std::to_string( expectedS ), std::to_string( actualS ) );
    printLine( "a:", std::to_string( expectedA ), std::to_string( actualA ) );
    printLine( "x:", std::to_string( expectedX ), std::to_string( actualX ) );
    printLine( "y:", std::to_string( expectedY ), std::to_string( actualY ) );
    printLine( "p:", std::to_string( expectedP ), std::to_string( actualP ) );

    // Blank line and RAM header
    std::cout << '\n' << "RAM" << '\n';

    // Print RAM entries
    for ( const auto& ramEntry : jsonData[state]["ram"] )
    {
        uint16_t address = ramEntry[0];
        uint8_t  expectedValue = ramEntry[1];
        uint8_t  actualValue = cpu.Read( address );

        // Format address
        oss << std::hex << std::setw( 4 ) << std::setfill( '0' ) << address;
        std::string addressStr = oss.str();
        oss.str( "" );
        oss.clear();

        // Format expected value
        oss << std::hex << std::setw( 2 ) << std::setfill( '0' ) << int( expectedValue );
        std::string expectedValueStr = oss.str();
        oss.str( "" );
        oss.clear();

        // Format actual value
        oss << std::hex << std::setw( 2 ) << std::setfill( '0' ) << int( actualValue );
        std::string actualValueStr = oss.str();
        oss.str( "" );
        oss.clear();

        // Print RAM line
        std::cout << addressStr << ": ";
        std::cout << std::setw( valueWidth ) << expectedValueStr;
        std::cout << std::setw( valueWidth ) << actualValueStr << '\n';
    }

    std::cout << "--------------------------------" << '\n';
    std::cout << '\n';
}
// Test case function for each test in the JSON array

void CPUTest::RunTestCase( const json& testCase )  // NOLINT
{
    // Initialize CPU
    cpu.Reset();

    // Load initial state from JSON
    LoadStateFromJson( testCase, "initial" );

    // Ensure loaded values match JSON values
    EXPECT_EQ( cpu.GetPC(), u16( testCase["initial"]["pc"] ) );
    EXPECT_EQ( cpu.GetA(), testCase["initial"]["a"] );
    EXPECT_EQ( cpu.GetX(), testCase["initial"]["x"] );
    EXPECT_EQ( cpu.GetY(), testCase["initial"]["y"] );
    EXPECT_EQ( cpu.GetS(), testCase["initial"]["s"] );
    EXPECT_EQ( cpu.GetP(), testCase["initial"]["p"] );
    for ( const auto& ramEntry : testCase["initial"]["ram"] )
    {
        uint16_t address = ramEntry[0];
        uint8_t  value = ramEntry[1];
        EXPECT_EQ( cpu.Read( address ), value );
    }

    // Print initial state
    if ( verbose )
    {
        std::cout << "Running Test: " << testCase["name"] << '\n';
        PrintCPUState( testCase, "initial" );
    }

    // Execute instructions
    cpu.FetchDecodeExecute();

    // Print final state
    if ( verbose )
    {
        PrintCPUState( testCase, "final" );
    }

    // Ensure final values match JSON values
    EXPECT_EQ( cpu.GetPC(), u16( testCase["final"]["pc"] ) )
        << "PC mismatch: Expected " << std::hex << std::setw( 4 ) << std::setfill( '0' )
        << u16( testCase["final"]["pc"] ) << ", but got " << cpu.GetPC();
    EXPECT_EQ( cpu.GetA(), u8( testCase["final"]["a"] ) )
        << "A mismatch: Expected " << u8( testCase["final"]["a"] ) << ", but got " << cpu.GetA();
    EXPECT_EQ( cpu.GetX(), u8( testCase["final"]["x"] ) )
        << "X mismatch: Expected " << u8( testCase["final"]["x"] ) << ", but got " << cpu.GetX();
    EXPECT_EQ( cpu.GetY(), u8( testCase["final"]["y"] ) )
        << "Y mismatch: Expected " << u8( testCase["final"]["y"] ) << ", but got " << cpu.GetY();
    EXPECT_EQ( cpu.GetS(), u8( testCase["final"]["s"] ) )
        << "S mismatch: Expected " << u8( testCase["final"]["s"] ) << ", but got " << cpu.GetS();
    EXPECT_EQ( cpu.GetP(), u8( testCase["final"]["p"] ) )
        << "P mismatch: Expected " << ParseStatus( u8( testCase["final"]["p"] ) ) << ", but got "
        << ParseStatus( cpu.GetP() );

    for ( const auto& ramEntry : testCase["final"]["ram"] )
    {
        uint16_t address = ramEntry[0];
        uint8_t  expectedValue = ramEntry[1];
        uint8_t  actualValue = cpu.Read( address );
        EXPECT_EQ( actualValue, expectedValue )
            << "RAM mismatch at address " << std::hex << address << ": Expected "
            << int( expectedValue ) << ", but got " << int( actualValue );
    }
}

auto CPUTest::ExtractTestsFromJson( const std::string& path ) -> json
{
    std::ifstream jsonFile( path );
    if ( !jsonFile.is_open() )
    {
        throw std::runtime_error( "Could not open test file: " + path );
    }
    json testCases;
    jsonFile >> testCases;

    if ( !testCases.is_array() )
    {
        throw std::runtime_error( "Expected an array of test cases in JSON file" );
    }
    return testCases;
}

void CPUTest::PrintTestStartMsg( const std::string& testName )
{
    std::cout << "---------- " << testName << " Tests ---------" << '\n';
}

void CPUTest::PrintTestEndMsg( const std::string& testName )
{
    std::cout << "---------- " << testName << " Tests Complete ---------" << '\n';
    std::cout << '\n';
}

auto CPUTest::ParseStatus( u8 status ) -> std::string
{
    std::string statusLabel = "NV-BDIZC";
    std::string flags;
    for ( int i = 7; i >= 0; i-- )
    {
        flags += ( status & ( 1 << i ) ) != 0 ? "1" : "0";
    }
    return statusLabel + " " + flags;
}
// NOLINTEND(cppcoreguidelines-owning-memory, cppcoreguidelines-avoid-non-const-global-variables,
// modernize-use-trailing-return-type)
