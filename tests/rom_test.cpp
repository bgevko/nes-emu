#include <gtest/gtest.h>
#include "bus.h"
#include "cpu.h"
#include "cartridge.h"

TEST( RomTests, CPUOfficial )
{
    Bus bus;
    CPU cpu( &bus );

    // Create a shared pointer to Cartridge
    std::shared_ptr<Cartridge> cartridge = std::make_shared<Cartridge>( "tests/roms/nestest.nes" );

    // Pass the shared pointer to LoadCartridge
    bus.LoadCartridge( cartridge );

    cpu.Reset();
    cpu.SetProgramCounter( 0xC000 ); // Set to 0xC000 to bypass PPU checks

    // Loop until the test completes successfully
    while ( true )
    {
        cpu.Tick(); // Run one CPU cycle

        // Read the status code from memory locations 0x02 and 0x03
        uint8_t error_code = bus.Read( 0x02 );
        uint8_t error_byte = bus.Read( 0x03 );

        // Check if the test has failed by looking at the error code
        if ( error_code != 0x00 )
        {
            std::cout << "Test failed at PC: " << std::hex << cpu.GetProgramCounter()
                      << ", Error Code: " << std::hex << (int) error_code
                      << ", Error Byte: " << (int) error_byte << '\n';
            FAIL() << "Test failed at error code " << std::hex << (int) error_code;
        }

        // Check if the test has completed successfully (when the PC reaches 0xC66E)
        if ( cpu.GetProgramCounter() == 0xC66E )
        {
            std::cout << "Test completed successfully.\n";
            break;
        }
    }
}

// Main function to run all tests
int main( int argc, char **argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
