#include <gtest/gtest.h>
#include "bus.h"
#include "cpu.h"
#include "cartridge.h"
#include <fstream>
#include <regex>
#include <vector>
#include <iostream>
#include <memory>

// Utility function to convert numbers to hexadecimal strings with leading zeros
auto toHex( uint32_t num, uint8_t width ) -> std::string
{
    std::string hex_str( width, '0' );
    const char  hex_chars[] = "0123456789ABCDEF";
    for ( int i = width - 1; i >= 0; --i, num >>= 4 )
    {
        hex_str[i] = hex_chars[num & 0xF];
    }
    return hex_str;
}

// Structure to hold expected CPU state from the log
struct ExpectedLine
{
    uint16_t             address;
    std::vector<uint8_t> opcode; // Changed to vector to handle multiple opcode bytes
    uint8_t              a;
    uint8_t              x;
    uint8_t              y;
    uint8_t              p;
    uint8_t              sp;
    uint64_t             cycles;
};

// Test case for CPU Official Nestest
TEST( RomTests, CPUOfficial )
{
    Bus bus;
    CPU cpu( &bus );

    // Create a shared pointer to Cartridge
    std::shared_ptr<Cartridge> cartridge = std::make_shared<Cartridge>( "tests/roms/nestest.nes" );

    // Pass the shared pointer to LoadCartridge
    bus.LoadCartridge( cartridge );

    cpu.Reset();

    // Open the log file for writing disassembly data
    std::ofstream log( "tests/logs/my_nestest.log" );
    if ( !log.is_open() )
    {
        std::cerr << "Failed to open my_nestest.log for writing\n";
        FAIL() << "Cannot open log file.";
        return;
    }

    // Open the nestest.log file for reading expected CPU states
    std::ifstream nestest_log( "tests/logs/nestest.log" );
    if ( !nestest_log.is_open() )
    {
        std::cerr << "Failed to open nestest.log for reading\n";
        FAIL() << "Cannot open nestest.log file.";
        return;
    }

    // Define an improved regex pattern to handle variable-length opcodes
    std::regex line_pattern(
        R"(^([A-F0-9]{4})\s+([A-F0-9]{2})\s.{39}A:([A-F0-9]{2})\s*X:([A-F0-9]{2})\s*Y:([A-F0-9]{2})\s*P:([A-F0-9]{2})\s*SP:([A-F0-9]{2})\s*.{12}CYC:(\d+))",
        std::regex_constants::ECMAScript );

    std::vector<ExpectedLine> expected_lines; // Vector to store parsed expected lines

    std::string line;
    size_t      line_number = 0; // To keep track of line numbers for debugging

    // Read and parse each line from nestest.log
    while ( std::getline( nestest_log, line ) )
    {
        ++line_number;
        std::smatch match;
        if ( std::regex_match( line, match, line_pattern ) )
        {
            try
            {
                // Ensure all groups are matched (total 8 groups: 1 full match + 7 capture groups)
                if ( match.size() != 9 )
                {
                    throw std::runtime_error( "Regex groups missing data" );
                }

                // Extract and convert the data
                ExpectedLine entry{};
                entry.address = static_cast<uint16_t>( std::stoul( match[1].str(), nullptr, 16 ) );

                // Parse opcode bytes
                std::vector<uint8_t> opcode_bytes;
                std::istringstream   opcode_stream( match[2].str() );
                std::string          byte_str;
                while ( opcode_stream >> byte_str )
                {
                    uint8_t byte = static_cast<uint8_t>( std::stoul( byte_str, nullptr, 16 ) );
                    opcode_bytes.push_back( byte );
                }
                entry.opcode = opcode_bytes;

                entry.a = static_cast<uint8_t>( std::stoul( match[3].str(), nullptr, 16 ) );
                entry.x = static_cast<uint8_t>( std::stoul( match[4].str(), nullptr, 16 ) );
                entry.y = static_cast<uint8_t>( std::stoul( match[5].str(), nullptr, 16 ) );
                entry.p = static_cast<uint8_t>( std::stoul( match[6].str(), nullptr, 16 ) );
                entry.sp = static_cast<uint8_t>( std::stoul( match[7].str(), nullptr, 16 ) );
                entry.cycles = static_cast<uint64_t>( std::stoul( match[8].str(), nullptr, 10 ) );

                expected_lines.push_back( entry ); // Store the parsed entry
            }
            catch ( const std::exception &e )
            {
                std::cerr << "Failed to parse line " << line_number << ": " << line
                          << "\nException: " << e.what() << '\n';
                FAIL() << "Parsing failed at line " << line_number;
                return;
            }
        }
        else
        {
            std::cerr << "Regex did not match line " << line_number << ": " << line << '\n';
            // Depending on requirements, you can choose to fail immediately or continue
            // Here, we'll choose to continue to parse the rest of the file
        }
    }

    // Check if any expected lines were parsed
    if ( expected_lines.empty() )
    {
        std::cerr << "No expected lines were parsed. Check your regex and input file.\n";
        FAIL() << "No parsed lines.";
        return;
    }

    std::cout << "Total expected lines parsed: " << expected_lines.size() << '\n';

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
        // Log disassembly
        log << cpu.DisassembleAtPC() << '\n';

        const auto &expected = expected_lines[line_index];

        // Compare each CPU register/state with expected values
        if ( cpu.GetProgramCounter() != expected.address )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": PC mismatch\n";
            std::cerr << "Expected: " << toHex( expected.address, 4 ) << "    "
                      << "Actual: " << toHex( cpu.GetProgramCounter(), 4 ) << '\n';
        }

        if ( cpu.GetAccumulator() != expected.a )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": A mismatch\n";
            std::cerr << "Expected: " << toHex( expected.a, 2 ) << "    "
                      << "Actual: " << toHex( cpu.GetAccumulator(), 2 ) << '\n';
        }

        if ( cpu.GetXRegister() != expected.x )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": X mismatch\n";
            std::cerr << "Expected: " << toHex( expected.x, 2 ) << "    "
                      << "Actual: " << toHex( cpu.GetXRegister(), 2 ) << '\n';
        }

        if ( cpu.GetYRegister() != expected.y )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": Y mismatch\n";
            std::cerr << "Expected: " << toHex( expected.y, 2 ) << "    "
                      << "Actual: " << toHex( cpu.GetYRegister(), 2 ) << '\n';
        }

        if ( cpu.GetStatusRegister() != expected.p )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": P mismatch\n";
            std::cerr << "Expected: " << toHex( expected.p, 2 ) << "    "
                      << "Actual: " << toHex( cpu.GetStatusRegister(), 2 ) << '\n';
        }

        if ( cpu.GetStackPointer() != expected.sp )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": SP mismatch\n";
            std::cerr << "Expected: " << toHex( expected.sp, 2 ) << "    "
                      << "Actual: " << toHex( cpu.GetStackPointer(), 2 ) << '\n';
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

        cpu.Tick(); // Run one CPU cycle
        line_index++;

        // Optional: Add a condition to prevent infinite loops or excessively long tests
        if ( line_index >= expected_lines.size() )
        {
            std::cerr << "Reached end of expected lines without completing the test.\n";
            break;
        }
    }

    if ( !did_fail )
    {
        std::cout << "Test completed successfully.\n";
    }

    log.close();
}

// Main function to run all tests
int main( int argc, char **argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
