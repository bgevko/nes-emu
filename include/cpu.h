// cpu.h
#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <vector>

using u8 = uint8_t;
using u16 = uint16_t;
using s8 = int8_t;

// CPU state with optional values to make testing partial states easier.
struct CPUState
{
    std::optional<u16>                       pc = std::nullopt;   // Program Counter
    std::optional<u8>                        a = std::nullopt;    // Accumulator
    std::optional<u8>                        x = std::nullopt;    // X Register
    std::optional<u8>                        y = std::nullopt;    // Y Register
    std::optional<u8>                        s = std::nullopt;    // Stack Pointer
    std::optional<u8>                        p = std::nullopt;    // Status Register
    std::optional<std::array<u8, 64 * 1024>> ram = std::nullopt;  // Memory
};

class CPU
{
   public:
    CPU();

    // Memory
    std::array<u8, 64 * 1024> ram;

    // Registers
    u16 pc;
    u8  a, x, y, s, p;

    // Opcodes table
    using OpcodeHandler = void ( * )( CPU& );
    std::array<OpcodeHandler, 256> opcodeTable;

    template <void ( CPU::*Op )()>
    void opcodeHandler( CPU& cpu )
    {
        ( cpu.*Op )();
    }
    template <void ( CPU::*Op )( void ( CPU::* )() ), void ( CPU::*addressingMode )()>
    void opcodeHandler( CPU& cpu )
    {
        ( cpu.*Op )( addressingMode );
    }

    // CPU Methods
    void Reset( CPUState state = {} );
    void FetchDecodeExecute();

    u8   Read( u16 address ) const;
    void Write( u16 address, u8 data );

    // Helpers
    void LoadProgram( const std::vector<u8>& data, u16 startAddress = 0x8000 );
    void PrintMemory( u16 start, u16 end = 0x0000 ) const;
    void PrintRegisters() const;

    // helper globals
    bool halt = false;

    // Addressing modes
    u16 IMM();   // IMM
    u16 ZPG();   // Zero Page
    u16 ABS();   // ABS
    u16 ABSX();  // Absolute x
    u16 ABSY();  // Absolute y
    u16 ZPGX();  // Zero Page x
    u16 ZPGY();  // Zero Page y
    u16 IND();   // Indirect
    u16 INDX();  // Indirect x
    u16 INDY();  // Indirect y
    u16 REL();   // Relative

   private:
    // Statuses
    enum Status : u8
    {
        Carry = 1 << 0,             // 0b00000001
        Zero = 1 << 1,              // 0b00000010
        InterruptDisable = 1 << 2,  // 0b00000100
        Decimal = 1 << 3,           // 0b00001000
        Break = 1 << 4,             // 0b00010000
        Unused = 1 << 5,            // 0b00100000
        Overflow = 1 << 6,          // 0b01000000
        Negative = 1 << 7,          // 0b10000000
    };

    // Opcodes
    void BRK();
    void LD( u16 ( CPU::*addressingMode )(), u8& reg );
    void STA( u16 ( CPU::*addressingMode )() );
    void AND( u16 ( CPU::*addressingMode )() );

    // helpers
    std::string GetStatusString();
    void        SetZeroAndNegativeFlags( u8 value );
    void        Push( u8 value );
    u8          Pop();
};
