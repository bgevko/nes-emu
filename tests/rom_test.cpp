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

void loadStateFromLogline( const string &logline, const regex &pattern, int nFields, CPU &cpu, PPU &ppu );

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
        u16 ppu_cycles;
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
        entry.ppu_cycles = static_cast<u16>( stoul( match[6], nullptr, 10 ) );
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
    regex mesen_log_pattern(
        R"(^([A-F0-9]{4}).*?A:\d{2}([A-F0-9]{2}).*?:\d{2}([A-F0-9]{2}).*?:\d{2}([A-F0-9]{2}).*?:\d{2}([A-F0-9]{2}).*?:(.{8}).*?:(-*\d+).*?:(\d+).*?:(\d+))",
        regex_constants::ECMAScript );

    // Pattern for local log output
    regex local_log_pattern(
        R"(^([A-F0-9]{4}):.*?a: ([A-F0-9]{2}).*?: ([A-F0-9]{2}).*?: ([A-F0-9]{2}).*?: ([A-F0-9]{2}).*?: [A-F0-9]{2}  (.{8}).*?: (-*\d+).*?: (\d+).*?: (\d+))",
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

    size_t start_line = 0;

    struct LogInfo
    {
        u16    pc;
        u8     a;
        u8     x;
        u8     y;
        u8     sp;
        string p;
        u16    ppu_cycles;
        s16    scanline;
        u64    cpu_cycles;
    };
    while ( std::getline( mesen_log, line ) )
    {
        // Skip ahead to the starting line
        if ( line_num < start_line )
        {
            line_num++;
            cpu.DecodeExecute();
            continue;
        }

        /*
        ################################
        ||                            ||
        ||    Parse Expected Output   ||
        ||                            ||
        ################################
        */
        LogInfo            expected{};
        utils::MatchResult match = utils::parseLogLine( line, mesen_log_pattern, 8 );
        expected.pc = static_cast<u16>( stoul( match[0], nullptr, 16 ) );
        expected.a = static_cast<u8>( stoul( match[1], nullptr, 16 ) );
        expected.x = static_cast<u8>( stoul( match[2], nullptr, 16 ) );
        expected.y = static_cast<u8>( stoul( match[3], nullptr, 16 ) );
        expected.sp = static_cast<u8>( stoul( match[4], nullptr, 16 ) );
        expected.p = match[5];
        expected.scanline = static_cast<s16>( stoul( match[6], nullptr, 10 ) );
        expected.ppu_cycles = static_cast<u16>( stoul( match[7], nullptr, 10 ) );
        expected.cpu_cycles = static_cast<u64>( stoull( match[8], nullptr, 10 ) );

        /*
        ################################
        ||                            ||
        ||  Compare to actual output  ||
        ||                            ||
        ################################
        */

        // color codes
        const std::string red = "\033[31m";
        const std::string green = "\033[32m";
        const std::string reset = "\033[0m";

        // Run one CPU cycle
        cpu.EnableTracelog();
        cpu.DecodeExecute();
        std::string trace_line = cpu.GetTrace();
        actual_output << trace_line << '\n';

        LogInfo            actual{};
        utils::MatchResult trace_match = utils::parseLogLine( trace_line, local_log_pattern, 8 );
        actual.pc = static_cast<u16>( stoul( trace_match[0], nullptr, 16 ) );
        actual.a = static_cast<u8>( stoul( trace_match[1], nullptr, 16 ) );
        actual.x = static_cast<u8>( stoul( trace_match[2], nullptr, 16 ) );
        actual.y = static_cast<u8>( stoul( trace_match[3], nullptr, 16 ) );
        actual.sp = static_cast<u8>( stoul( trace_match[4], nullptr, 16 ) );
        actual.p = trace_match[5];
        actual.scanline = static_cast<s16>( stoul( trace_match[6], nullptr, 10 ) );
        actual.ppu_cycles = static_cast<u16>( stoul( trace_match[7], nullptr, 10 ) );
        actual.cpu_cycles = stoull( trace_match[8], nullptr, 10 );

        // Compare Mesen's trace log to our own
        if ( actual.pc != expected.pc )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "PC mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << utils::toHex( expected.pc, 4 ) << "    "
                 << "Actual: " << utils::toHex( actual.pc, 4 ) << '\n';
            cerr << '\n';
        }
        if ( actual.a != expected.a )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "A mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << utils::toHex( expected.a, 2 ) << "    "
                 << "Actual: " << utils::toHex( actual.a, 2 ) << '\n';
            cerr << '\n';
        }
        if ( actual.x != expected.x )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "X mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << utils::toHex( expected.x, 2 ) << "    "
                 << "Actual: " << utils::toHex( actual.x, 2 ) << '\n';
            cerr << '\n';
        }
        if ( actual.y != expected.y )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "Y mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << utils::toHex( expected.y, 2 ) << "    "
                 << "Actual: " << utils::toHex( actual.y, 2 ) << '\n';
            cerr << '\n';
        }
        if ( actual.sp != expected.sp )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "SP mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << utils::toHex( expected.sp, 2 ) << "    "
                 << "Actual: " << utils::toHex( actual.sp, 2 ) << '\n';
            cerr << '\n';
        }
        if ( actual.p != expected.p )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "P mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << expected.p << "    "
                 << "Actual: " << actual.p << '\n';
            cerr << '\n';
        }
        if ( actual.scanline != expected.scanline )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "Scanline mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << expected.scanline << "    "
                 << "Actual: " << actual.scanline << '\n';
            cerr << '\n';
        }
        if ( actual.ppu_cycles != expected.ppu_cycles )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "PPU Cycles mismatch at line " << line_num << reset << '\n';

            cerr << "Expected: " << expected.ppu_cycles << "    "
                 << "Actual: " << actual.ppu_cycles << '\n';
            cerr << '\n';
        }
        if ( actual.cpu_cycles != expected.cpu_cycles )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << "CPU Cycles mismatch at line " << line_num << reset << '\n';
            cerr << "Expected: " << expected.cpu_cycles << "    "
                 << "Actual: " << actual.cpu_cycles << '\n';
            cerr << '\n';
        }
        if ( did_fail )
        {
            string actual_line = cpu.GetTrace();

            loadStateFromLogline( line, mesen_log_pattern, 8, cpu, ppu );
            string expected_line = cpu.LogLineAtPC();

            // Print the state of the cpu at failure
            std::cerr << "\n";
            std::cerr << "Actual Vs. Expected:\n";
            std::cerr << red << actual_line << reset << '\n';
            std::cerr << green << expected_line << reset << '\n';
            std::cerr << "\n";
            FAIL();
            break;
        }

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

/*
################################
||                            ||
||        Test Helpers        ||
||                            ||
################################
*/

void loadStateFromLogline( const string &logline, const regex &pattern, int nFields, CPU &cpu, PPU &ppu )
{
    utils::MatchResult trace_match = utils::parseLogLine( logline, pattern, nFields );
    cpu.SetProgramCounter( static_cast<u16>( stoul( trace_match[0], nullptr, 16 ) ) );
    cpu.SetAccumulator( static_cast<u8>( stoul( trace_match[1], nullptr, 16 ) ) );
    cpu.SetXRegister( static_cast<u8>( stoul( trace_match[2], nullptr, 16 ) ) );
    cpu.SetYRegister( static_cast<u8>( stoul( trace_match[3], nullptr, 16 ) ) );
    cpu.SetStackPointer( static_cast<u8>( stoul( trace_match[4], nullptr, 16 ) ) );
    ppu.SetScanline( static_cast<s16>( stoul( trace_match[6], nullptr, 10 ) ) );
    ppu.SetCycles( static_cast<u16>( stoul( trace_match[7], nullptr, 10 ) ) );
    cpu.SetCycles( stoull( trace_match[8], nullptr, 10 ) );

    // Status
    // Mesen status come in form of string: nv--dIZC
    // We'll convert it to a byte
    std::string status_str = trace_match[5];
    uint8_t     status = 0;
    for ( int i = 0; i < 8; ++i )
    {
        // ignore '-' and lower case letters
        if ( status_str[i] != '-' && ( std::isupper( status_str[i] ) == 1 ) )
        {
            status |= 1 << ( 7 - i );
        }
    }
    cpu.SetStatusRegister( status | 0x20 );
}
