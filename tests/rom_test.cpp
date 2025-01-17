#include <gtest/gtest.h>
#include "utils.h"
#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "cartridge.h"
#include <fstream>
#include <regex>
#include <thread>
#include <vector>
#include <iostream>
#include <memory>
using namespace std;

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

    // Open the nestest.log file for reading expected CPU states
    ifstream nestest_log( "tests/logs/nestest-log.txt" );
    if ( !nestest_log.is_open() )
    {
        cerr << "Failed to open nestest-log.txt for reading\n";
        FAIL() << "Cannot open nestest log file.";
        return;
    }

    // Define a bit of regex magic to grab groups of hex bytes
    regex line_pattern(
        R"(^([A-F0-9]{4})\s+([A-F0-9]{2})\s.{39}A:([A-F0-9]{2})\s*X:([A-F0-9]{2})\s*Y:([A-F0-9]{2})\s*P:([A-F0-9]{2})\s*SP:([A-F0-9]{2})\s*.{12}CYC:(\d+))",
        regex_constants::ECMAScript );

    // Read in the expected lines into a vector, which we will use to compare against our own
    // execution
    vector<ExpectedLine> expected_lines;

    string line;
    size_t line_number = 0;
    while ( getline( nestest_log, line ) )
    {
        ++line_number;
        smatch match;
        // here comes the regex magic
        if ( regex_match( line, match, line_pattern ) )
        {
            try
            {
                if ( match.size() != 9 )
                {
                    throw runtime_error( "Regex groups missing data" );
                }

                // Address
                ExpectedLine entry{};
                entry.address = static_cast<uint16_t>( stoul( match[1].str(), nullptr, 16 ) );

                // Parse opcode bytes
                vector<uint8_t> opcode_bytes;
                istringstream   opcode_stream( match[2].str() );
                string          byte_str;
                while ( opcode_stream >> byte_str )
                {
                    uint8_t byte = static_cast<uint8_t>( stoul( byte_str, nullptr, 16 ) );
                    opcode_bytes.push_back( byte );
                }
                entry.opcode = opcode_bytes;

                // Registers and the rest of the data
                entry.a = static_cast<uint8_t>( stoul( match[3].str(), nullptr, 16 ) );
                entry.x = static_cast<uint8_t>( stoul( match[4].str(), nullptr, 16 ) );
                entry.y = static_cast<uint8_t>( stoul( match[5].str(), nullptr, 16 ) );
                entry.p = static_cast<uint8_t>( stoul( match[6].str(), nullptr, 16 ) );
                entry.sp = static_cast<uint8_t>( stoul( match[7].str(), nullptr, 16 ) );
                entry.cycles = static_cast<uint64_t>( stoul( match[8].str(), nullptr, 10 ) );

                expected_lines.push_back( entry );
            }
            catch ( const exception &e )
            {
                cerr << "Failed to parse line " << line_number << ": " << line
                     << "\nException: " << e.what() << '\n';
                FAIL() << "Parsing failed at line " << line_number;
                return;
            }
        }
        else
        {
            cerr << "Regex did not match line " << line_number << ": " << line << '\n';
        }
    }

    if ( expected_lines.empty() )
    {
        std::cerr << "No expected lines were parsed. Check your regex and input file.\n";
        FAIL() << "No parsed lines.";
        return;
    }

    // Initialize CPU state with the first expected entry
    cpu.SetProgramCounter( expected_lines[0].address );
    cpu.SetAccumulator( expected_lines[0].a );
    cpu.SetXRegister( expected_lines[0].x );
    cpu.SetYRegister( expected_lines[0].y );
    cpu.SetStatusRegister( expected_lines[0].p );
    cpu.SetStackPointer( expected_lines[0].sp );
    cpu.SetCycles( expected_lines[0].cycles );

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

        if ( cpu.GetProgramCounter() != expected.address )
        {
            did_fail = true;
            cerr << ( line_index + 1 ) << ": PC mismatch\n";
            cerr << "Expected: " << utils::toHex( expected.address, 4 ) << "    "
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

        if ( cpu.GetCycles() != expected.cycles )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": Cycles mismatch\n";
            std::cerr << "Expected: " << expected.cycles << "    "
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

    if ( !did_fail )
    {
        std::cout << "Nestest passed.\n";
    }
    else
    {
        std::cerr << "Nestest failed.\n";
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
    ofstream log( "tests/output/my_instr_test_v5.txt" );
    if ( !log.is_open() )
    {
        cerr << "Failed to open output/my_instr_test_v5.txt for writing\n";
        FAIL() << "Cannot open log file.";
        return;
    }

    bool   test_passed = false;
    string previous_output;

    int test_index = 0;
    int last_line = 0;
    int instr_count = 0;
    // Emulation loop
    while ( true )
    {
        log << cpu.LogLineAtPC() << '\n';
        // cout << cpu.LogLineAtPC() << '\n';
        // if ( cpu.GetCycles() == 86954 - 1 )
        // {
        //     cout << "made it here." << '\n';
        // }
        cpu.DecodeExecute();
        instr_count++;

        // Read test status from $6000
        uint8_t status = cpu.Read( 0x6000 );

        // If the status is 0x81 => we must wait 100ms, then reset CPU
        if ( status == 0x81 )
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
            cpu.Reset();
        }

        if ( cpu.GetProgramCounter() == 0xEC5B )
        {
            last_line++;
        }

        if ( last_line == 10 )
        {
            break;
        }

        // Stop after x amount of instructions, uncomment as needed.
        // if ( instr_count == 100000 )
        // {
        //     break;
        // }

        // Stop after x condition, uncomment as needed.
        // if ( ppu.GetScanline() == 2 )
        // {
        //     break;
        // }

        // Read the output starting from 6004 until zero byte
        while ( status != 0 )
        {
            u8 output_char = cpu.Read( 0x6004 + test_index );
            if ( output_char == 0 )
            {
                break;
            }
            // print to the console in real time
            cout << (char) output_char;
            test_index++;
        }
    }
    log.close();

    if ( test_passed )
    {
        std::cout << "Test completed successfully.\n";
    }
    else
    {
        FAIL() << "InstructionTestV5 failed.";
    }
}

int main( int argc, char **argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
