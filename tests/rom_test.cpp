#include <gtest/gtest.h>
#include "utils.h"
#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "cartridge.h"
#include <fstream>
#include <regex>
#include <vector>
#include <iostream>
#include <memory>

// Structure to hold expected CPU state from the log
struct ExpectedLine
{
    uint16_t        address;
    vector<uint8_t> opcode; // Changed to vector to handle multiple opcode bytes
    uint8_t         a;
    uint8_t         x;
    uint8_t         y;
    uint8_t         p;
    uint8_t         sp;
    uint64_t        cycles;
};

/*
################################################
||                                            ||
||                 Nestest.nes                ||
||                                            ||
################################################
*/
TEST( RomTests, Nestest )
{
    PPU ppu{};
    Bus bus( &ppu );
    CPU cpu( &bus );

    // Create a shared pointer to Cartridge
    shared_ptr<Cartridge> cartridge = make_shared<Cartridge>( "tests/roms/nestest.nes" );

    // Pass the shared pointer to LoadCartridge
    bus.LoadCartridge( cartridge );

    cpu.Reset();

    // Open the log file for writing disassembly data
    ofstream log( "tests/output/my_nestest-log.txt" );
    if ( !log.is_open() )
    {
        cerr << "Failed to open output/my_nestest-log.txt for writing\n";
        FAIL() << "Cannot open log file.";
        return;
    }

    // Pattern for nestest log output
    regex pattern(
        R"(^([A-F0-9]{4}).*?A:([A-F0-9]{2}).*?X:([A-F0-9]{2}).*?Y:([A-F0-9]{2}).*?P:([A-F0-9]{2}).*?SP:([A-F0-9]{2}).*?,\s*(\d+).*?CYC:(\d+))",
        regex_constants::ECMAScript );

    utils::MatchResults matches = utils::parseLog( "tests/logs/nestest-log.txt", pattern, 8 );

    struct NestestLogInfo
    {
        u16 pc;
        u8  a;
        u8  x;
        u8  y;
        u8  p;
        u8  sp;
        s16 ppu_cycles;
        u64 cpu_cycles;
    };
    std::vector<NestestLogInfo> expected_lines;
    for ( const auto &match : matches )
    {
        NestestLogInfo entry{};
        entry.pc = static_cast<u16>( stoul( match[0], nullptr, 16 ) );
        entry.a = static_cast<u8>( stoul( match[1], nullptr, 16 ) );
        entry.x = static_cast<u8>( stoul( match[2], nullptr, 16 ) );
        entry.y = static_cast<u8>( stoul( match[3], nullptr, 16 ) );
        entry.p = static_cast<u8>( stoul( match[4], nullptr, 16 ) );
        entry.sp = static_cast<u8>( stoul( match[5], nullptr, 16 ) );
        entry.ppu_cycles = static_cast<s16>( stoul( match[6], nullptr, 10 ) );
        entry.cpu_cycles = stoull( match[7], nullptr, 10 );
        expected_lines.push_back( entry );
    }

    // Initialize CPU state with the first expected entry
    cpu.SetProgramCounter( expected_lines[0].pc );
    cpu.SetAccumulator( expected_lines[0].a );
    cpu.SetXRegister( expected_lines[0].x );
    cpu.SetYRegister( expected_lines[0].y );
    cpu.SetStatusRegister( expected_lines[0].p );
    cpu.SetStackPointer( expected_lines[0].sp );
    cpu.SetCycles( expected_lines[0].cpu_cycles );

    size_t line_index = 0;
    bool   did_fail = false;

    // Execute the CPU until the end of the test ROM

    while ( line_index < expected_lines.size() )
    {
        // Save output to our own log file
        log << cpu.LogLineAtPC() << '\n';

        // Get the expected line
        const auto &expected = expected_lines[line_index];

        // Compare all the relevant values with expected

        if ( cpu.GetProgramCounter() != expected.pc )
        {
            did_fail = true;
            cerr << ( line_index + 1 ) << ": PC mismatch\n";
            cerr << "Expected: " << utils::toHex( expected.pc, 4 ) << "    "
                 << "Actual: " << utils::toHex( cpu.GetProgramCounter(), 4 ) << '\n';
        }

        if ( cpu.GetAccumulator() != expected.a )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": A mismatch\n";
            std::cerr << "Expected: " << utils::toHex( expected.a, 2 ) << "    "
                      << "Actual: " << utils::toHex( cpu.GetAccumulator(), 2 ) << '\n';
        }

        if ( cpu.GetXRegister() != expected.x )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": X mismatch\n";
            std::cerr << "Expected: " << utils::toHex( expected.x, 2 ) << "    "
                      << "Actual: " << utils::toHex( cpu.GetXRegister(), 2 ) << '\n';
        }

        if ( cpu.GetYRegister() != expected.y )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": Y mismatch\n";
            std::cerr << "Expected: " << utils::toHex( expected.y, 2 ) << "    "
                      << "Actual: " << utils::toHex( cpu.GetYRegister(), 2 ) << '\n';
        }

        if ( cpu.GetStatusRegister() != expected.p )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": P mismatch\n";
            std::cerr << "Expected: " << utils::toHex( expected.p, 2 ) << "    "
                      << "Actual: " << utils::toHex( cpu.GetStatusRegister(), 2 ) << '\n';
        }

        if ( cpu.GetStackPointer() != expected.sp )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": SP mismatch\n";
            std::cerr << "Expected: " << utils::toHex( expected.sp, 2 ) << "    "
                      << "Actual: " << utils::toHex( cpu.GetStackPointer(), 2 ) << '\n';
        }

        if ( cpu.GetCycles() != expected.cpu_cycles )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": Cycles mismatch\n";
            std::cerr << "Expected: " << expected.cpu_cycles << "    "
                      << "Actual: " << cpu.GetCycles() << '\n';
        }

        if ( did_fail )
        {

            FAIL() << "Mismatch detected at line " << ( line_index + 1 );
            break;
        }

        cpu.DecodeExecute(); // Run one CPU cycle
        line_index++;
    }

    if ( did_fail )
    {
        std::cerr << "Nestest failed.\n";
    }
    else
    {

        std::cout << "Nestest passed.\n";
    }

    log.close();
}

/*
################################################
||                                            ||
||              InstructionTestV5             ||
||                                            ||
################################################
*/

TEST( RomTests, InstructionTestV5 )
{
    PPU ppu{};
    Bus bus( &ppu );
    CPU cpu( &bus );

    //     // Load the v5 instruction test ROM
    shared_ptr<Cartridge> cartridge = make_shared<Cartridge>( "tests/roms/instr_test-v5.nes" );
    bus.LoadCartridge( cartridge );
    cpu.Reset();

    //     // Open a log file to capture output
    ofstream actual_output( "tests/output/my_instr_test_v5.txt" );
    if ( !actual_output.is_open() )
    {
        cerr << "Failed to open output/my_instr_test_v5.txt for writing\n";
        FAIL() << "Cannot open file for writing.";
        return;
    }

    // Pattern for default mesen log output
    regex pattern(
        R"(^([A-F0-9]{4}).*?:\d{2}([A-F0-9]{2}).*?:\d{2}([A-F0-9]{2}).*?:\d{2}([A-F0-9]{2}).*?:\d{2}([A-F0-9]{2}).*?:(.{8}).*?:(\d+).*?:(\d+).*?:(\d+))",
        regex_constants::ECMAScript );

    // Instead of parsing the entire log file, which may be 1+ million lines
    // We can compare it line by line, in real time
    std::string   filename = "tests/logs/mesen_v5.txt";
    std::ifstream mesen_log( filename );
    if ( !mesen_log.is_open() )
    {
        throw std::runtime_error( "Failed to open " + filename + " for reading" );
    }
    std::string line;
    size_t      line_num = 0;
    bool        did_fail = false;

    struct InstrV5LogInfo
    {
        uint16_t pc;
        uint8_t  a;
        uint8_t  x;
        uint8_t  y;
        uint8_t  p;
        uint8_t  sp;
        uint16_t ppu_cycles;
        uint16_t scanline;
        uint64_t cpu_cycles;
    };
    while ( std::getline( mesen_log, line ) )
    {
        /*
        ################################
        ||                            ||
        ||    Parse Expected Output   ||
        ||                            ||
        ################################
        */
        InstrV5LogInfo     expected{};
        utils::MatchResult match = utils::parseLogLine( line, pattern, 8 );
        expected.pc = static_cast<uint16_t>( stoul( match[0], nullptr, 16 ) );
        expected.a = static_cast<uint8_t>( stoul( match[1], nullptr, 16 ) );
        expected.x = static_cast<uint8_t>( stoul( match[2], nullptr, 16 ) );
        expected.y = static_cast<uint8_t>( stoul( match[3], nullptr, 16 ) );
        expected.sp = static_cast<uint8_t>( stoul( match[4], nullptr, 16 ) );
        expected.scanline = static_cast<uint16_t>( stoul( match[6], nullptr, 10 ) );

        // Mesen shows +1 CPU cycle and +2 PPU cycle more than what's in its internal state..
        expected.ppu_cycles = static_cast<uint16_t>( stoul( match[7], nullptr, 10 ) ) - 2;
        expected.cpu_cycles = stoull( match[8], nullptr, 10 ) - 1;

        // Status
        // Mesen status come in form of string: nv--dIZC
        // We'll convert it to a byte
        std::string status_str = match[5];
        uint8_t     status = 0;
        for ( int i = 0; i < 8; ++i )
        {
            // ignore '-', 'I' and lower case
            if ( status_str[i] != '-' && ( std::isupper( status_str[i] ) == 1 ) && status_str[i] != 'I' )
            {
                status |= 1 << ( 7 - i );
            }
        }
        expected.p = status | 0x20; // Unused bit to 1

        /*
        ################################
        ||                            ||
        ||  Compare to actual output  ||
        ||                            ||
        ################################
        */
        // Log current state to output file
        actual_output << cpu.LogLineAtPC() << '\n';

        // color codes
        const std::string red = "\033[31m";
        const std::string reset = "\033[0m";

        // compare actual vs expected state
        if ( cpu.GetProgramCounter() != expected.pc )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "PC mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << utils::toHex( expected.pc, 4 ) << "    "
                 << "Actual: " << utils::toHex( cpu.GetProgramCounter(), 4 ) << '\n';
            cerr << '\n';
        }
        if ( cpu.GetAccumulator() != expected.a )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "A mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << utils::toHex( expected.a, 2 ) << "    "
                 << "Actual: " << utils::toHex( cpu.GetAccumulator(), 2 ) << '\n';
            cerr << '\n';
        }
        if ( cpu.GetXRegister() != expected.x )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "X mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << utils::toHex( expected.x, 2 ) << "    "
                 << "Actual: " << utils::toHex( cpu.GetXRegister(), 2 ) << '\n';
            cerr << '\n';
        }
        if ( cpu.GetYRegister() != expected.y )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "Y mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << utils::toHex( expected.y, 2 ) << "    "
                 << "Actual: " << utils::toHex( cpu.GetYRegister(), 2 ) << '\n';
            cerr << '\n';
        }
        if ( cpu.GetStackPointer() != expected.sp )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "SP mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << utils::toHex( expected.sp, 2 ) << "    "
                 << "Actual: " << utils::toHex( cpu.GetStackPointer(), 2 ) << '\n';
            cerr << '\n';
        }
        if ( cpu.GetStatusRegister() != expected.p )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "P mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << utils::toHex( expected.p, 2 ) << "    "
                 << "Actual: " << utils::toHex( cpu.GetStatusRegister(), 2 ) << '\n';
            cerr << '\n';
        }
        if ( ppu.GetScanline() != expected.scanline )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "Scanline mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << expected.scanline << "    "
                 << "Actual: " << ppu.GetScanline() << '\n';
            cerr << '\n';
        }
        if ( ppu.GetCycles() != expected.ppu_cycles )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "PPU Cycles mismatch at line " << line_num << reset << '\n';

            cerr << "Expected: " << expected.ppu_cycles << "    "
                 << "Actual: " << ppu.GetCycles() << '\n';
            cerr << '\n';
        }
        if ( cpu.GetCycles() != expected.cpu_cycles )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "CPU Cycles mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << expected.cpu_cycles << "    "
                 << "Actual: " << cpu.GetCycles() << '\n';
            cerr << '\n';
        }
        if ( did_fail )
        {
            // Print the state of the cpu at failure
            std::cerr << "\n";
            std::cerr << "CPU STATE:\n";
            std::cerr << red << cpu.LogLineAtPC() << reset << '\n';
            std::cerr << "\n";
            FAIL();
            break;
        }

        // Run one CPU cycle
        cpu.DecodeExecute();
        line_num++;
    }

    actual_output.close();

    if ( did_fail )
    {
        std::cerr << "InstructionTestV5 failed.\n";
    }
    else
    {

        std::cout << "InstructionTestV5 completed successfully.\n";
    }
}

int main( int argc, char **argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
