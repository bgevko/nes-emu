#include <gtest/gtest.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "cpu.h"
#include "json.hpp"

using json = nlohmann::json;

void        LoadStateFromJson( CPU& cpu, const json& j, const std::string& state );
void        PrintCPUState( const CPU& cpu, const json& j, const std::string& state );
void        RunTestCase( const json& test_case );
void        PrintTestStartMsg( const std::string& test_name );
void        PrintTestEndMsg( const std::string& test_name );
json        ExtractTestsFromJson( const std::string& path );
std::string ParseStatus( u8 status );

// TODO: Set as directive later..
bool VERBOSE = false;

TEST( ImmediateAddressing, IMM )
{
    std::string test_name = "Immediate Addressing Mode IMM";
    PrintTestStartMsg( test_name );
    CPU cpu;
    cpu.pc = 0x8000;
    EXPECT_EQ( cpu.pc, 0x8000 );
    u16 addr = cpu.IMM();
    EXPECT_EQ( addr, 0x8000 );
    EXPECT_EQ( cpu.pc, 0x8001 );
    PrintTestEndMsg( test_name );
}

TEST( ZeroPageAddressing, ZPG )
{
    std::string test_name = "Zero Page Addressing Mode ZPG";
    PrintTestStartMsg( test_name );
    CPU cpu;
    cpu.pc = 0x0000;
    cpu.Write( 0x0000, 0x42 );
    u16 addr = cpu.ZPG();
    EXPECT_EQ( addr, 0x42 );
    EXPECT_EQ( cpu.pc, 0x0001 );
    PrintTestEndMsg( test_name );
}

TEST( ZeroPageXAddressing, ZPGX )
{
    std::string test_name = "Zero Page X Addressing Mode ZPGX";
    PrintTestStartMsg( test_name );
    CPU cpu;
    cpu.pc = 0x0000;
    cpu.x = 0x1;

    // Write the zero page address at the pc location
    cpu.Write( cpu.pc, 0x80 );  // Zero page address 0x80
    u8 zp_addr = cpu.Read( cpu.pc );
    u8 expected_addr = ( zp_addr + cpu.x ) & 0xFF;

    // Write a test value at the expected address
    cpu.Write( expected_addr, 0x42 );

    u16 addr = cpu.ZPGX();
    EXPECT_EQ( addr, expected_addr )
        << "Expected " << int( expected_addr ) << ", but got " << int( addr );
    EXPECT_EQ( cpu.Read( addr ), 0x42 ) << "Expected 0x42, but got " << int( cpu.Read( addr ) );
    EXPECT_EQ( cpu.pc, 0x0001 );
    PrintTestEndMsg( test_name );
}

TEST( ZeroPageYAddressing, ZPGY )
{
    std::string test_name = "Zero Page Y Addressing Mode ZPGY";
    PrintTestStartMsg( test_name );
    CPU cpu;
    cpu.pc = 0x0000;
    cpu.y = 0x1;

    // Write the zero page address at the pc location
    cpu.Write( cpu.pc, 0x80 );  // Zero page address 0x80
    u8 zp_addr = cpu.Read( cpu.pc );
    u8 expected_addr = ( zp_addr + cpu.x ) & 0xFF;

    // Write a test value at the expected address
    cpu.Write( expected_addr, 0x42 );

    u16 addr = cpu.ZPGX();
    EXPECT_EQ( addr, expected_addr )
        << "Expected " << int( expected_addr ) << ", but got " << int( addr );
    EXPECT_EQ( cpu.Read( addr ), 0x42 ) << "Expected 0x42, but got " << int( cpu.Read( addr ) );
    EXPECT_EQ( cpu.pc, 0x0001 );
    PrintTestEndMsg( test_name );
}

TEST( AbsoluteAddressing, ABS )
{
    std::string test_name = "Absolute Addressing Mode ABS";
    PrintTestStartMsg( test_name );
    CPU cpu;
    cpu.pc = 0x0000;
    cpu.Write( 0x0000, 0x42 );
    cpu.Write( 0x0001, 0x24 );
    u16 addr = cpu.ABS();
    EXPECT_EQ( addr, 0x2442 );
    EXPECT_EQ( cpu.pc, 0x0002 );
    PrintTestEndMsg( test_name );
}

TEST( IndirectAddressing, IND )
{
    std::string test_name = "Indirect Addressing Mode IND";
    PrintTestStartMsg( test_name );
    CPU cpu;
    cpu.pc = 0x0000;

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
    EXPECT_EQ( cpu.pc, 0x0002 );

    PrintTestEndMsg( test_name );
}

TEST( IndirectAddressingBug, IND )
{
    std::string test_name = "Indirect Addressing Mode Bug IND";
    PrintTestStartMsg( test_name );
    CPU cpu;
    cpu.pc = 0x0000;

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
    EXPECT_EQ( cpu.pc, 0x0002 );

    PrintTestEndMsg( test_name );
}

TEST( IndirectXAddressing, INDX )
{
    std::string test_name = "Indirect X Addressing Mode INDX";
    PrintTestStartMsg( test_name );
    CPU cpu;
    cpu.pc = 0x0000;
    cpu.x = 0x10;

    // Write the operand at the pc location
    u8 operand = 0x23;
    cpu.Write( 0x0000, operand );
    u8  ptr = ( operand + cpu.x ) & 0xFF;
    u16 effectiveAddr = 0xABCD;

    // Write the effective addressto zero-page memory
    cpu.Write( ptr, effectiveAddr & 0xFF );               // Low byte
    cpu.Write( ( ptr + 1 ) & 0xFF, effectiveAddr >> 8 );  // High byte

    // Write a test value at the effective address
    cpu.Write( effectiveAddr, 0x42 );

    u16 addr = cpu.INDX();
    EXPECT_EQ( addr, effectiveAddr )
        << "Expected " << std::hex << effectiveAddr << ", but got " << addr;
    EXPECT_EQ( cpu.Read( addr ), 0x42 ) << "Expected 0x42, but got " << int( cpu.Read( addr ) );
    EXPECT_EQ( cpu.pc, 0x0001 );

    PrintTestEndMsg( test_name );
}

TEST( IndirectYAddressing, INDY )
{
    std::string test_name = "Indirect Y Addressing Mode INDY";
    PrintTestStartMsg( test_name );

    CPU cpu;
    cpu.pc = 0x0000;
    cpu.y = 0x10;

    u8 operand = 0x23;
    cpu.Write( 0x0000, operand );
    u8  ptr = operand;
    u16 effectiveAddr = 0xAB00;

    // Write the effective address to zero page memory
    cpu.Write( ptr, effectiveAddr & 0xFF );    // Low byte
    cpu.Write( ptr + 1, effectiveAddr >> 8 );  // High byte

    u16 finalAddr = effectiveAddr + cpu.y;
    cpu.Write( finalAddr, 0x42 );  // Write a test value at the final address
    u16 addr = cpu.INDY();
    EXPECT_EQ( addr, finalAddr ) << "Expected " << std::hex << finalAddr << ", but got " << addr;
    EXPECT_EQ( cpu.Read( addr ), 0x42 ) << "Expected 0x42, but got " << int( cpu.Read( addr ) );
    EXPECT_EQ( cpu.pc, 0x0001 );

    PrintTestEndMsg( test_name );
}

TEST( RelativeAddressing, REL )
{
    std::string test_name = "Relative Addressing Mode REL";
    PrintTestStartMsg( test_name );
    CPU cpu;
    cpu.pc = 0x1000;

    // Write a relative address of -5 at the pc location
    cpu.Write( cpu.pc, 0xFB );
    u16 backBranch = cpu.REL();
    EXPECT_EQ( backBranch, 0x0FFB ) << "Expected 0x0FFB, but got " << std::hex << backBranch;
    EXPECT_EQ( cpu.pc, 0x1001 );

    // Write a relative address of +5 at the pc location
    /* cpu.pc = 0x1000; */
    /* cpu.Write( cpu.pc, 0x05 ); */
    /* u16 forwardBranch = cpu.REL(); */
    /* EXPECT_EQ( forwardBranch, 0x1005 ) << "Expected 0x1005, but got " << std::hex <<
     * forwardBranch; */
    /* EXPECT_EQ( cpu.pc, 0x1001 ); */
}

TEST( x00, Break )
{
    std::string test_name = "00 BRK";
    PrintTestStartMsg( test_name );
    json test_cases = ExtractTestsFromJson( "tests/HARTE/a9.json" );
    for ( const auto& test_case : test_cases )
    {
        RunTestCase( test_case );
    }
    PrintTestEndMsg( test_name );
}

TEST( x8D, STA_Absolute )
{
    std::string test_name = "8D STA Absolute";
    PrintTestStartMsg( test_name );
    json test_cases = ExtractTestsFromJson( "tests/HARTE/8d.json" );
    for ( const auto& test_case : test_cases )
    {
        RunTestCase( test_case );
    }
    PrintTestEndMsg( test_name );
}

TEST( xA0, LDY_Immediate )
{
    std::string test_name = "A0 LDY Immediate";
    PrintTestStartMsg( test_name );
    json test_cases = ExtractTestsFromJson( "tests/HARTE/a0.json" );
    for ( const auto& test_case : test_cases )
    {
        RunTestCase( test_case );
    }
    PrintTestEndMsg( test_name );
}

TEST( xA1, LDA_Xindirect )
{
    std::string test_name = "A1 LDA X-Indirect";
    PrintTestStartMsg( test_name );
    json test_cases = ExtractTestsFromJson( "tests/HARTE/a1.json" );
    for ( const auto& test_case : test_cases )
    {
        RunTestCase( test_case );
    }
    PrintTestEndMsg( test_name );
}

TEST( xA2, LDX_Immediate )
{
    std::string test_name = "A2 LDX Immediate";
    PrintTestStartMsg( test_name );
    json test_cases = ExtractTestsFromJson( "tests/HARTE/a2.json" );
    for ( const auto& test_case : test_cases )
    {
        RunTestCase( test_case );
    }
    PrintTestEndMsg( test_name );
}

TEST( xA4, LDY_ZeroPage )
{
    std::string test_name = "A4 LDY ZeroPage";
    PrintTestStartMsg( test_name );
    json test_cases = ExtractTestsFromJson( "tests/HARTE/a4.json" );
    for ( const auto& test_case : test_cases )
    {
        RunTestCase( test_case );
    }
    PrintTestEndMsg( test_name );
}

TEST( xA5, LDA_ZeroPage )
{
    std::string test_name = "A5 LDA ZeroPage";
    PrintTestStartMsg( test_name );
    json test_cases = ExtractTestsFromJson( "tests/HARTE/a5.json" );
    for ( const auto& test_case : test_cases )
    {
        RunTestCase( test_case );
    }
    PrintTestEndMsg( test_name );
}

TEST( xA6, LDX_ZeroPage )
{
    std::string test_name = "A6 LDX ZeroPage";
    PrintTestStartMsg( test_name );
    json test_cases = ExtractTestsFromJson( "tests/HARTE/a6.json" );
    for ( const auto& test_case : test_cases )
    {
        RunTestCase( test_case );
    }
    PrintTestEndMsg( test_name );
}

TEST( xAC, LDY_Absolute )
{
    std::string test_name = "AC LDY Absolute";
    PrintTestStartMsg( test_name );
    json test_cases = ExtractTestsFromJson( "tests/HARTE/ac.json" );
    for ( const auto& test_case : test_cases )
    {
        RunTestCase( test_case );
    }
    PrintTestEndMsg( test_name );
}

TEST( xAD, LDA_Absolute )
{
    std::string test_name = "AD LDA Absolute";
    PrintTestStartMsg( test_name );
    json test_cases = ExtractTestsFromJson( "tests/HARTE/ad.json" );
    for ( const auto& test_case : test_cases )
    {
        RunTestCase( test_case );
    }
    PrintTestEndMsg( test_name );
}

TEST( xAE, LDX_Absolute )
{
    std::string test_name = "AE LDX Absolute";
    PrintTestStartMsg( test_name );
    json test_cases = ExtractTestsFromJson( "tests/HARTE/ae.json" );
    for ( const auto& test_case : test_cases )
    {
        RunTestCase( test_case );
    }
    PrintTestEndMsg( test_name );
}

TEST( xA9, LDA_Immediate )
{
    std::string test_name = "A9 LDA Immediate";
    PrintTestStartMsg( test_name );
    json test_cases = ExtractTestsFromJson( "tests/HARTE/a9.json" );
    for ( const auto& test_case : test_cases )
    {
        RunTestCase( test_case );
    }
    PrintTestEndMsg( test_name );
}

TEST( xB6, LDX_ZeroPageY )
{
    std::string test_name = "B6 LDX ZeroPageY";
    PrintTestStartMsg( test_name );
    /* json test_cases = ExtractTestsFromJson( "tests/HARTE/b6.json" ); */
    json test_cases = ExtractTestsFromJson( "tests/small.json" );

    for ( const auto& test_case : test_cases )
    {
        RunTestCase( test_case );
    }
    PrintTestEndMsg( test_name );
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------
// ----------------------- Helper Functions -----------------------
// ----------------------------------------------------------------

// Load CPU state from JSON
void LoadStateFromJson( CPU& cpu, const json& j, const std::string& state )
{
    cpu.pc = u16( j[state]["pc"] );
    cpu.a = j[state]["a"];
    cpu.x = j[state]["x"];
    cpu.y = j[state]["y"];
    cpu.s = j[state]["s"];
    cpu.p = j[state]["p"];
    for ( const auto& ram_entry : j[state]["ram"] )
    {
        uint16_t address = ram_entry[0];
        uint8_t  value = ram_entry[1];
        cpu.Write( address, value );
    }
}

// Helper function to print CPU state
void PrintCPUState( const CPU& cpu, const json& j, const std::string& state )
{
    std::cout << "----------" << state << " State----------" << std::endl;
    std::cout << "pc: " << std::hex << std::setw( 4 ) << std::setfill( '0' ) << cpu.pc << std::endl;
    std::cout << "s: " << std::dec << int( cpu.s ) << std::endl;
    std::cout << "a: " << int( cpu.a ) << std::endl;
    std::cout << "x: " << int( cpu.x ) << std::endl;
    std::cout << "y: " << int( cpu.y ) << std::endl;
    std::cout << "p: " << int( cpu.p ) << std::endl;
    std::cout << std::endl;
    std::cout << "RAM:" << std::endl;
    for ( const auto& ram_entry : j[state]["ram"] )
    {
        uint16_t address = ram_entry[0];
        uint8_t  value = cpu.Read( address );
        std::cout << std::hex << std::setw( 4 ) << std::setfill( '0' ) << address << ": "
                  << std::hex << std::setw( 2 ) << std::setfill( '0' ) << int( value ) << std::endl;
    }
    std::cout << "--------------------------------" << std::endl;
    std::cout << std::endl;
}

// Test case function for each test in the JSON array
void RunTestCase( const json& test_case )
{
    // Initialize CPU
    CPU cpu;
    cpu.Reset();

    // Load initial state from JSON
    LoadStateFromJson( cpu, test_case, "initial" );

    // Ensure loaded values match JSON values
    EXPECT_EQ( cpu.pc, u16( test_case["initial"]["pc"] ) );
    EXPECT_EQ( cpu.a, test_case["initial"]["a"] );
    EXPECT_EQ( cpu.x, test_case["initial"]["x"] );
    EXPECT_EQ( cpu.y, test_case["initial"]["y"] );
    EXPECT_EQ( cpu.s, test_case["initial"]["s"] );
    EXPECT_EQ( cpu.p, test_case["initial"]["p"] );
    for ( const auto& ram_entry : test_case["initial"]["ram"] )
    {
        uint16_t address = ram_entry[0];
        uint8_t  value = ram_entry[1];
        EXPECT_EQ( cpu.Read( address ), value );
    }

    // Print initial state
    if ( VERBOSE )
    {
        std::cout << "Running Test: " << test_case["name"] << std::endl;
        PrintCPUState( cpu, test_case, "initial" );
    }

    // Execute instructions
    cpu.FetchDecodeExecute();

    // Print final state
    if ( VERBOSE )
    {
        PrintCPUState( cpu, test_case, "final" );
    }

    // Ensure final values match JSON values
    EXPECT_EQ( cpu.pc, u16( test_case["final"]["pc"] ) )
        << "PC mismatch: Expected " << std::hex << std::setw( 4 ) << std::setfill( '0' )
        << u16( test_case["final"]["pc"] ) << ", but got " << cpu.pc;
    EXPECT_EQ( cpu.a, u8( test_case["final"]["a"] ) )
        << "A mismatch: Expected " << u8( test_case["final"]["a"] ) << ", but got " << cpu.a;
    EXPECT_EQ( cpu.x, u8( test_case["final"]["x"] ) )
        << "X mismatch: Expected " << std::hex << std::setw( 2 ) << std::setfill( '0' )
        << u8( test_case["final"]["x"] ) << ", but got " << cpu.x;
    EXPECT_EQ( cpu.y, u8( test_case["final"]["y"] ) )
        << "Y mismatch: Expected " << std::hex << std::setw( 2 ) << std::setfill( '0' )
        << u8( test_case["final"]["y"] ) << ", but got " << cpu.y;
    EXPECT_EQ( cpu.s, u8( test_case["final"]["s"] ) )
        << "S mismatch: Expected " << std::hex << std::setw( 2 ) << std::setfill( '0' )
        << u8( test_case["final"]["s"] ) << ", but got " << cpu.s;
    EXPECT_EQ( cpu.p, u8( test_case["final"]["p"] ) )
        << "PC mismatch: Expected " << ParseStatus( u8( test_case["final"]["p"] ) ) << ", but got "
        << ParseStatus( cpu.p );

    for ( const auto& ram_entry : test_case["final"]["ram"] )
    {
        uint16_t address = ram_entry[0];
        uint8_t  expected_value = ram_entry[1];
        uint8_t  actual_value = cpu.Read( address );
        EXPECT_EQ( actual_value, expected_value )
            << "RAM mismatch at address " << std::hex << address << ": Expected "
            << int( expected_value ) << ", but got " << int( actual_value );
    }
}

json ExtractTestsFromJson( const std::string& path )
{
    std::ifstream json_file( path );
    if ( !json_file.is_open() )
    {
        throw std::runtime_error( "Could not open test file: " + path );
    }
    json test_cases;
    json_file >> test_cases;

    if ( !test_cases.is_array() )
    {
        throw std::runtime_error( "Expected an array of test cases in JSON file" );
    }
    return test_cases;
}

void PrintTestStartMsg( const std::string& test_name )
{
    std::cout << "---------- " << test_name << " Tests ---------" << std::endl;
}

void PrintTestEndMsg( const std::string& test_name )
{
    std::cout << "---------- " << test_name << " Tests Complete ---------" << std::endl;
    std::cout << std::endl;
}

std::string ParseStatus( u8 status )
{
    std::string statusLabel = "NV-BDIZC";
    std::string flags = "";
    for ( int i = 7; i >= 0; i-- )
    {
        flags += ( status & ( 1 << i ) ) ? "1" : "0";
    }
    return statusLabel + " " + flags;
}
