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
bool runDecimalModeTests = false;

class CPUTest : public ::testing::Test
{
  protected:
    CPU cpu; // NOLINT
    // Helper methods
    void        LoadStateFromJson( const json &jsonData, const std::string &state );
    void        PrintCPUState( const json &jsonData, const std::string &state );
    std::string GetCPUStateString( const json &jsonData, const std::string &state );
    void        RunTestCase( const json &testCase );
    static void PrintTestStartMsg( const std::string &testName );
    static void PrintTestEndMsg( const std::string &testName );
    static auto ExtractTestsFromJson( const std::string &path ) -> json;
    static auto ParseStatus( u8 status ) -> std::string;
};

//------------------------------------------------------------------
//-------------------- ADRRESSING MODE TESTS -----------------------
//------------------------------------------------------------------

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
    cpu.Write( cpu.GetPC(), 0x80 ); // Zero page address 0x80
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
    cpu.Write( cpu.GetPC(), 0x80 ); // Zero page address 0x80
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
    cpu.Write( ptr, effectiveAddr & 0xFF );              // Low byte
    cpu.Write( ( ptr + 1 ) & 0xFF, effectiveAddr >> 8 ); // High byte

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
    cpu.Write( ptr, effectiveAddr & 0xFF );   // Low byte
    cpu.Write( ptr + 1, effectiveAddr >> 8 ); // High byte

    u16 finalAddr = effectiveAddr + cpu.GetY();
    cpu.Write( finalAddr, 0x42 ); // Write a test value at the final address
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

//------------------------------------------------------------------
//-------------------- INSTRUCTION TESTS ---------------------------
//------------------------------------------------------------------

// Each test name in in the format:
// x<last two digits of opcode>_<mnemonic>_<addressing mode>
// Examples:
// x00_BRK_Implied - BRK instruction with Implied addressing mode
// x01_ORA_IndirectX - ORA instruction with IndirectX addressing mode
// etc.
// To isolate a test, use the test.sh script:
// ./test.sh "CPUTest.x00" # Will run only the BRK test
// ./test.sh "CPUTest.x01" # Will run only the ORA test
#define CPU_TEST( opcode_hex, mnemonic, addr_mode, filename )                                      \
    TEST_F( CPUTest, x##opcode_hex##_##mnemonic##_##addr_mode )                                    \
    {                                                                                              \
        std::string testName = #opcode_hex " " #mnemonic " " #addr_mode;                           \
        PrintTestStartMsg( testName );                                                             \
        json testCases = ExtractTestsFromJson( "tests/HARTE/" filename );                          \
        for ( const auto &testCase : testCases )                                                   \
        {                                                                                          \
            RunTestCase( testCase );                                                               \
        }                                                                                          \
        PrintTestEndMsg( testName );                                                               \
    }

CPU_TEST( 00, BRK, Implied, "00.json" );
CPU_TEST( 01, ORA, IndirectX, "01.json" );
CPU_TEST( 05, ORA, ZeroPage, "05.json" );
CPU_TEST( 06, ASL, ZeroPage, "06.json" );
CPU_TEST( 08, PHP, Implied, "08.json" );
CPU_TEST( 09, ORA, Immediate, "09.json" );
CPU_TEST( 0A, ASL, Accumulator, "0a.json" );
CPU_TEST( 0D, ORA, Absolute, "0d.json" );
CPU_TEST( 0E, ASL, Absolute, "0e.json" );
CPU_TEST( 10, BPL, Relative, "10.json" );
CPU_TEST( 11, ORA, IndirectY, "11.json" );
CPU_TEST( 15, ORA, ZeroPageX, "15.json" );
CPU_TEST( 16, ASL, ZeroPageX, "16.json" );
CPU_TEST( 18, CLC, Implied, "18.json" );
CPU_TEST( 19, ORA, AbsoluteY, "19.json" );
CPU_TEST( 1D, ORA, AbsoluteX, "1d.json" );
CPU_TEST( 1E, ASL, AbsoluteX, "1e.json" );
CPU_TEST( 20, JSR, Absolute, "20.json" );
CPU_TEST( 21, AND, IndirectX, "21.json" );
CPU_TEST( 24, BIT, ZeroPage, "24.json" );
CPU_TEST( 25, AND, ZeroPage, "25.json" );
CPU_TEST( 26, ROL, ZeroPage, "26.json" );
CPU_TEST( 28, PLP, Implied, "28.json" );
CPU_TEST( 29, AND, Immediate, "29.json" );
CPU_TEST( 2A, ROL, Accumulator, "2a.json" );
CPU_TEST( 2C, BIT, Absolute, "2c.json" );
CPU_TEST( 2D, AND, Absolute, "2d.json" );
CPU_TEST( 2E, ROL, Absolute, "2e.json" );
CPU_TEST( 30, BMI, Relative, "30.json" );
CPU_TEST( 31, AND, IndirectY, "31.json" );
CPU_TEST( 35, AND, ZeroPageX, "35.json" );
CPU_TEST( 36, ROL, ZeroPageX, "36.json" );
CPU_TEST( 38, SEC, Implied, "38.json" );
CPU_TEST( 39, AND, AbsoluteY, "39.json" );
CPU_TEST( 3D, AND, AbsoluteX, "3d.json" );
CPU_TEST( 3E, ROL, AbsoluteX, "3e.json" );
CPU_TEST( 40, RTI, Implied, "40.json" );
CPU_TEST( 41, EOR, IndirectX, "41.json" );
CPU_TEST( 45, EOR, ZeroPage, "45.json" );
CPU_TEST( 46, LSR, ZeroPage, "46.json" );
CPU_TEST( 48, PHA, Implied, "48.json" );
CPU_TEST( 49, EOR, Immediate, "49.json" );
CPU_TEST( 4A, LSR, Accumulator, "4a.json" );
CPU_TEST( 4C, JMP, Absolute, "4c.json" );
CPU_TEST( 4D, EOR, Absolute, "4d.json" );
CPU_TEST( 4E, LSR, Absolute, "4e.json" );
CPU_TEST( 50, BVC, Relative, "50.json" );
CPU_TEST( 51, EOR, IndirectY, "51.json" );
CPU_TEST( 55, EOR, ZeroPageX, "55.json" );
CPU_TEST( 56, LSR, ZeroPageX, "56.json" );
CPU_TEST( 58, CLI, Implied, "58.json" );
CPU_TEST( 59, EOR, AbsoluteY, "59.json" );
CPU_TEST( 5D, EOR, AbsoluteX, "5d.json" );
CPU_TEST( 5E, LSR, AbsoluteX, "5e.json" );
CPU_TEST( 60, RTS, Implied, "60.json" );
CPU_TEST( 61, ADC, IndirectX, "61.json" );
CPU_TEST( 65, ADC, ZeroPage, "65.json" );
CPU_TEST( 66, ROR, ZeroPage, "66.json" );
CPU_TEST( 68, PLA, Implied, "68.json" );
CPU_TEST( 69, ADC, Immediate, "69.json" );
CPU_TEST( 6A, ROR, Accumulator, "6a.json" );
CPU_TEST( 6C, JMP, Indirect, "6c.json" );
CPU_TEST( 6D, ADC, Absolute, "6d.json" );
CPU_TEST( 6E, ROR, Absolute, "6e.json" );
CPU_TEST( 70, BVS, Relative, "70.json" );
CPU_TEST( 71, ADC, IndirectY, "71.json" );
CPU_TEST( 75, ADC, ZeroPageX, "75.json" );
CPU_TEST( 76, ROR, ZeroPageX, "76.json" );
CPU_TEST( 78, SEI, Implied, "78.json" );
CPU_TEST( 79, ADC, AbsoluteY, "79.json" );
CPU_TEST( 7D, ADC, AbsoluteX, "7d.json" );
CPU_TEST( 7E, ROR, AbsoluteX, "7e.json" );
CPU_TEST( 81, STA, IndirectX, "81.json" );
CPU_TEST( 84, STY, ZeroPage, "84.json" );
CPU_TEST( 85, STA, ZeroPage, "85.json" );
CPU_TEST( 86, STX, ZeroPage, "86.json" );
CPU_TEST( 88, DEY, Implied, "88.json" );
CPU_TEST( 8A, TXA, Implied, "8a.json" );
CPU_TEST( 8C, STY, Absolute, "8c.json" );
CPU_TEST( 8D, STA, Absolute, "8d.json" );
CPU_TEST( 8E, STX, Absolute, "8e.json" );
CPU_TEST( 90, BCC, Relative, "90.json" );
CPU_TEST( 91, STA, IndirectY, "91.json" );
CPU_TEST( 94, STY, ZeroPageX, "94.json" );
CPU_TEST( 95, STA, ZeroPageX, "95.json" );
CPU_TEST( 96, STX, ZeroPageY, "96.json" );
CPU_TEST( 98, TYA, Implied, "98.json" );
CPU_TEST( 99, STA, AbsoluteY, "99.json" );
CPU_TEST( 9A, TXS, Implied, "9a.json" );
CPU_TEST( 9D, STA, AbsoluteX, "9d.json" );
CPU_TEST( A0, LDY, Immediate, "a0.json" );
CPU_TEST( A1, LDA, IndirectX, "a1.json" );
CPU_TEST( A2, LDX, Immediate, "a2.json" );
CPU_TEST( A4, LDY, ZeroPage, "a4.json" );
CPU_TEST( A5, LDA, ZeroPage, "a5.json" );
CPU_TEST( A6, LDX, ZeroPage, "a6.json" );
CPU_TEST( A8, TAY, Implied, "a8.json" );
CPU_TEST( A9, LDA, Immediate, "a9.json" );
CPU_TEST( AA, TAX, Implied, "aa.json" );
CPU_TEST( AC, LDY, Absolute, "ac.json" );
CPU_TEST( AD, LDA, Absolute, "ad.json" );
CPU_TEST( AE, LDX, Absolute, "ae.json" );
CPU_TEST( B0, BCS, Relative, "b0.json" );
CPU_TEST( B1, LDA, IndirectY, "b1.json" );
CPU_TEST( B4, LDY, ZeroPageX, "b4.json" );
CPU_TEST( B5, LDA, ZeroPageX, "b5.json" );
CPU_TEST( B6, LDX, ZeroPageY, "b6.json" );
CPU_TEST( B8, CLV, Implied, "b8.json" );
CPU_TEST( B9, LDA, AbsoluteY, "b9.json" );
CPU_TEST( BA, TSX, Implied, "ba.json" );
CPU_TEST( BC, LDY, AbsoluteX, "bc.json" );
CPU_TEST( BD, LDA, AbsoluteX, "bd.json" );
CPU_TEST( BE, LDX, AbsoluteY, "be.json" );
CPU_TEST( C0, CPY, Immediate, "c0.json" );
CPU_TEST( C1, CMP, IndirectX, "c1.json" );
CPU_TEST( C4, CPY, ZeroPage, "c4.json" );
CPU_TEST( C5, CMP, ZeroPage, "c5.json" );
CPU_TEST( C6, DEC, ZeroPage, "c6.json" );
CPU_TEST( C8, INY, Implied, "c8.json" );
CPU_TEST( C9, CMP, Immediate, "c9.json" );
CPU_TEST( CA, DEX, Implied, "ca.json" );
CPU_TEST( CC, CPY, Absolute, "cc.json" );
CPU_TEST( CD, CMP, Absolute, "cd.json" );
CPU_TEST( CE, DEC, Absolute, "ce.json" );
CPU_TEST( D0, BNE, Relative, "d0.json" );
CPU_TEST( D1, CMP, IndirectY, "d1.json" );
CPU_TEST( D5, CMP, ZeroPageX, "d5.json" );
CPU_TEST( D6, DEC, ZeroPageX, "d6.json" );
CPU_TEST( D8, CLD, Implied, "d8.json" );
CPU_TEST( D9, CMP, AbsoluteY, "d9.json" );
CPU_TEST( DD, CMP, AbsoluteX, "dd.json" );
CPU_TEST( DE, DEC, AbsoluteX, "de.json" );
CPU_TEST( E0, CPX, Immediate, "e0.json" );
CPU_TEST( E1, SBC, IndirectX, "e1.json" );
CPU_TEST( E4, CPX, ZeroPage, "e4.json" );
CPU_TEST( E5, SBC, ZeroPage, "e5.json" );
CPU_TEST( E6, INC, ZeroPage, "e6.json" );
CPU_TEST( E8, INX, Implied, "e8.json" );
CPU_TEST( E9, SBC, Immediate, "e9.json" );
CPU_TEST( EA, NOP, Implied, "ea.json" );
CPU_TEST( EC, CPX, Absolute, "ec.json" );
CPU_TEST( ED, SBC, Absolute, "ed.json" );
CPU_TEST( EE, INC, Absolute, "ee.json" );
CPU_TEST( F0, BEQ, Relative, "f0.json" );
CPU_TEST( F1, SBC, IndirectY, "f1.json" );
CPU_TEST( F5, SBC, ZeroPageX, "f5.json" );
CPU_TEST( F6, INC, ZeroPageX, "f6.json" );
CPU_TEST( F8, SED, Implied, "f8.json" );
CPU_TEST( F9, SBC, AbsoluteY, "f9.json" );
CPU_TEST( FD, SBC, AbsoluteX, "fd.json" );
CPU_TEST( FE, INC, AbsoluteX, "fe.json" );

//------------------------------------------------------------------
//-------------------- DECIMAL MODE TESTS --------------------------
//------------------------------------------------------------------
/* TEST_F( CPUTest, x61_ADC_IndirectX_Decimal ) */
/* { */
/*     if ( !runDecimalModeTests ) */
/*     { */
/*         GTEST_SKIP(); */
/*     } */
/*     std::string testName = "ADC IndirectX Decimal"; */
/*     PrintTestStartMsg( testName ); */
/*     json testCases = ExtractTestsFromJson( "tests/Harte/61_decimal.json" ); */
/*     for ( const auto& testCase : testCases ) */
/*     { */
/*         RunTestCase( testCase ); */
/*     } */
/*     PrintTestEndMsg( testName ); */
/* } */

//------------------------------------------------------------------
//-------------------- MAIN FUNCTION ------------------------------
//------------------------------------------------------------------
int main( int argc, char **argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------
// ----------------------- Helper Functions -----------------------
// ----------------------------------------------------------------

void CPUTest::LoadStateFromJson( const json &jsonData, const std::string &state )
{
    cpu.SetPC( u16( jsonData[state]["pc"] ) );
    cpu.SetA( jsonData[state]["a"] );
    cpu.SetX( jsonData[state]["x"] );
    cpu.SetY( jsonData[state]["y"] );
    cpu.SetS( jsonData[state]["s"] );
    cpu.SetP( jsonData[state]["p"] );
    for ( const auto &ramEntry : jsonData[state]["ram"] )
    {
        uint16_t address = ramEntry[0];
        uint8_t  value = ramEntry[1];
        cpu.Write( address, value );
    }
}

// Helper function to print CPU state
void CPUTest::PrintCPUState( const json &jsonData, const std::string &state )
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
        [&]( const std::string &label, const std::string &expected, const std::string &actual )
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
    for ( const auto &ramEntry : jsonData[state]["ram"] )
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

void CPUTest::RunTestCase( const json &testCase ) // NOLINT
{
    // Initialize CPU
    cpu.Reset();

    // Load initial state from JSON
    LoadStateFromJson( testCase, "initial" );

    std::string initialState = GetCPUStateString( testCase, "initial" );

    // Ensure loaded values match JSON values
    EXPECT_EQ( cpu.GetPC(), u16( testCase["initial"]["pc"] ) );
    EXPECT_EQ( cpu.GetA(), testCase["initial"]["a"] );
    EXPECT_EQ( cpu.GetX(), testCase["initial"]["x"] );
    EXPECT_EQ( cpu.GetY(), testCase["initial"]["y"] );
    EXPECT_EQ( cpu.GetS(), testCase["initial"]["s"] );
    EXPECT_EQ( cpu.GetP(), testCase["initial"]["p"] );
    for ( const auto &ramEntry : testCase["initial"]["ram"] )
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

    bool               testFailed = false; // Track if any test has failed
    std::ostringstream errorMessages;      // Accumulate error messages
                                           //
    // Ensure final values match JSON values
    /* EXPECT_EQ( cpu.GetPC(), u16( testCase["final"]["pc"] ) ) */
    /*     << "PC mismatch: Expected " << std::hex << std::setw( 4 ) << std::setfill( '0' ) */
    /*     << u16( testCase["final"]["pc"] ) << ", but got " << cpu.GetPC(); */
    /* EXPECT_EQ( cpu.GetA(), u8( testCase["final"]["a"] ) ) */
    /*     << "A mismatch: Expected " << u8( testCase["final"]["a"] ) << ", but got " << cpu.GetA();
     */
    /* EXPECT_EQ( cpu.GetX(), u8( testCase["final"]["x"] ) ) */
    /*     << "X mismatch: Expected " << u8( testCase["final"]["x"] ) << ", but got " << cpu.GetX();
     */
    /* EXPECT_EQ( cpu.GetY(), u8( testCase["final"]["y"] ) ) */
    /*     << "Y mismatch: Expected " << u8( testCase["final"]["y"] ) << ", but got " << cpu.GetY();
     */
    /* EXPECT_EQ( cpu.GetS(), u8( testCase["final"]["s"] ) ) */
    /*     << "S mismatch: Expected " << u8( testCase["final"]["s"] ) << ", but got " <<
     * cpu.GteetS();
     */
    /* EXPECT_EQ( cpu.GetP(), u8( testCase["final"]["p"] ) ) */
    /*     << "P mismatch: Expected " << ParseStatus( u8( testCase["final"]["p"] ) ) << ", but got "
     */
    /*     << ParseStatus( cpu.GetP() ); */
    if ( cpu.GetPC() != u16( testCase["final"]["pc"] ) )
    {
        testFailed = true;
        errorMessages << "PC ";
    }
    if ( cpu.GetA() != u8( testCase["final"]["a"] ) )
    {
        testFailed = true;
        errorMessages << "A ";
    }
    if ( cpu.GetX() != u8( testCase["final"]["x"] ) )
    {
        testFailed = true;
        errorMessages << "X ";
    }
    if ( cpu.GetY() != u8( testCase["final"]["y"] ) )
    {
        testFailed = true;
        errorMessages << "Y ";
    }
    if ( cpu.GetS() != u8( testCase["final"]["s"] ) )
    {
        testFailed = true;
        errorMessages << "S ";
    }
    if ( cpu.GetP() != u8( testCase["final"]["p"] ) )
    {
        testFailed = true;
        errorMessages << "P ";
    }

    for ( const auto &ramEntry : testCase["final"]["ram"] )
    {
        uint16_t address = ramEntry[0];
        uint8_t  expectedValue = ramEntry[1];
        uint8_t  actualValue = cpu.Read( address );
        /* EXPECT_EQ( actualValue, expectedValue ) */
        /*     << "RAM mismatch at address " << std::hex << address << ": Expected " */
        /*     << int( expectedValue ) << ", but got " << int( actualValue ); */
        if ( actualValue != expectedValue )
        {
            testFailed = true;
            errorMessages << "RAM ";
        }
    }

    std::string finalState = GetCPUStateString( testCase, "final" );
    // print initial and final state if there are any failures
    if ( testFailed )
    {
        std::cout << "Test Case: " << testCase["name"] << '\n';
        std::cout << "Failed: " << errorMessages.str() << '\n';
        std::cout << initialState << '\n';
        std::cout << finalState << '\n';
        std::cout << '\n';
        FAIL();
    }
}

std::string CPUTest::GetCPUStateString( const json &jsonData, const std::string &state )
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

    // Use ostringstream to collect output
    std::ostringstream output;

    // Print header
    output << "----------" << state << " State----------" << '\n';
    output << std::left << std::setw( labelWidth ) << "" << std::setw( valueWidth ) << "EXPECTED"
           << std::setw( valueWidth ) << "ACTUAL" << '\n';

    // Function to format and print a line
    auto printLine =
        [&]( const std::string &label, const std::string &expected, const std::string &actual )
    {
        auto toHexDecimalString = []( const std::string &value )
        {
            std::stringstream ss;
            int               intValue = std::strtol( value.c_str(), nullptr, 16 );

            // Print hex value with proper formatting, followed by decimal in parentheses
            ss << std::hex << std::uppercase << std::setw( 2 ) << std::setfill( '0' ) << intValue
               << " (" << std::dec << intValue << ")";

            return ss.str();
        };

        output << std::left << std::setw( labelWidth ) << label;
        output << std::setw( valueWidth ) << toHexDecimalString( expected );
        output << std::setw( valueWidth ) << toHexDecimalString( actual ) << '\n';
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
    output << std::dec << std::setfill( ' ' );

    // Print registers
    printLine( "pc:", expectedPCStr, actualPCStr );
    printLine( "s:", std::to_string( expectedS ), std::to_string( actualS ) );
    printLine( "a:", std::to_string( expectedA ), std::to_string( actualA ) );
    printLine( "x:", std::to_string( expectedX ), std::to_string( actualX ) );
    printLine( "y:", std::to_string( expectedY ), std::to_string( actualY ) );
    printLine( "p:", std::to_string( expectedP ), std::to_string( actualP ) );

    // Blank line and RAM header
    output << '\n' << "RAM" << '\n';

    // Print RAM entries
    for ( const auto &ramEntry : jsonData[state]["ram"] )
    {
        uint16_t address = ramEntry[0];
        uint8_t  expectedValue = ramEntry[1];
        uint8_t  actualValue = cpu.Read( address );

        // Helper lambda to format values as "HEX (DECIMAL)"
        auto formatValue = []( uint8_t value )
        {
            std::ostringstream oss;
            oss << std::hex << std::uppercase << std::setw( 2 ) << std::setfill( '0' )
                << static_cast<int>( value ) << " (" << std::dec << static_cast<int>( value )
                << ")";
            return oss.str();
        };

        // Format address as hex only (no decimal for addresses)
        std::ostringstream addressStream;
        addressStream << std::hex << std::setw( 4 ) << std::setfill( '0' ) << address;

        // Print formatted output
        output << addressStream.str() << ": ";
        output << std::setw( valueWidth ) << formatValue( expectedValue );
        output << std::setw( valueWidth ) << formatValue( actualValue ) << '\n';
    }

    output << "--------------------------------" << '\n';
    output << '\n';

    // Return the accumulated string
    return output.str();
}

auto CPUTest::ExtractTestsFromJson( const std::string &path ) -> json
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

void CPUTest::PrintTestStartMsg( const std::string &testName )
{
    std::cout << "---------- " << testName << " Tests ---------" << '\n';
}

void CPUTest::PrintTestEndMsg( const std::string &testName )
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
