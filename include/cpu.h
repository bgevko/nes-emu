// cpu.h
#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

using u8 = uint8_t;
using u16 = uint16_t;
using s8 = int8_t;

constexpr size_t kMemorySize = static_cast<size_t>( 64 * 1024 );
constexpr size_t numOpcodes = 256;
constexpr u16    defaultStartAddress = 0x8000;

// CPU state with optional values to make testing partial states easier.
struct CPUState
{
    std::optional<u16>                         pc = std::nullopt;      // Program Counter
    std::optional<u8>                          a = std::nullopt;       // Accumulator
    std::optional<u8>                          x = std::nullopt;       // X Register
    std::optional<u8>                          y = std::nullopt;       // Y Register
    std::optional<u8>                          s = std::nullopt;       // Stack Pointer
    std::optional<u8>                          p = std::nullopt;       // Status Register
    std::optional<std::array<u8, kMemorySize>> memory = std::nullopt;  // Memory
};

class CPU
{
   public:
    CPU();

    // Getters and Setters
    [[nodiscard]] auto GetPC() const -> u16;
    [[nodiscard]] auto GetA() const -> u8;
    [[nodiscard]] auto GetX() const -> u8;
    [[nodiscard]] auto GetY() const -> u8;
    [[nodiscard]] auto GetS() const -> u8;
    [[nodiscard]] auto GetP() const -> u8;
    [[nodiscard]] auto GetMemory() const -> const std::array<u8, kMemorySize>&;
    [[nodiscard]] auto IsHalted() const;

    void SetPC( u16 pcVal );
    void SetA( u8 aVal );
    void SetX( u8 xVal );
    void SetY( u8 yVal );
    void SetS( u8 sVal );
    void SetP( u8 pVal );
    void SetMemory( const std::array<u8, kMemorySize>& memory );
    void SetHalted( bool halt );

    // CPU Methods
    void Reset( CPUState state = {} );
    void FetchDecodeExecute();

    [[nodiscard]] auto Read( u16 address ) const -> u8;
    void               Write( u16 address, u8 data );

    // Helpers
    void LoadProgram( const std::vector<u8>& data, u16 startAddress = defaultStartAddress );
    void PrintMemory( u16 start, u16 end = 0x0000 ) const;
    void PrintRegisters() const;

    // Addressing modes
    auto IMP() -> u16;   // Implied
    auto IMM() -> u16;   // IMM
    auto ZPG() -> u16;   // Zero Page
    auto ABS() -> u16;   // ABS
    auto ABSX() -> u16;  // Absolute x
    auto ABSY() -> u16;  // Absolute y
    auto ZPGX() -> u16;  // Zero Page x
    auto ZPGY() -> u16;  // Zero Page y
    auto IND() -> u16;   // Indirect
    auto INDX() -> u16;  // Indirect x
    auto INDY() -> u16;  // Indirect y
    auto REL() -> u16;   // Relative

   private:
    // Registers
    u16 _pc;
    u8  _a, _x, _y, _s, _p;

    // Memory
    std::array<u8, kMemorySize> _memory;
    // friend class for testing

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

    // helper globals
    bool _halt = false;

    // Opcodes table
    using OpcodeHandler = void ( * )( CPU& );

    template <void ( CPU::*Op )()>
    void ExecuteOpcode( CPU& cpu )
    {
        ( cpu.*Op )();
    }
    template <void ( CPU::*Op )( void ( CPU::* )() ), void ( CPU::*addressingMode )()>
    void ExecuteOpcode( CPU& cpu )
    {
        ( cpu.*Op )( addressingMode );
    }
    std::array<OpcodeHandler, numOpcodes> _op;

    // Instruction Helpers
    void BRK();
    void LD( u16 ( CPU::*addressingMode )(), u8& reg );
    void ST( u16 ( CPU::*addressingMode )(), u8 reg );
    void Transfer( u8& src, u8& dest, bool updateFlags = true );
    void AND( u16 ( CPU::*addressingMode )() );
    void ORA( u16 ( CPU::*addressingMode )() );
    void EOR( u16 ( CPU::*addressingMode )() );
    void BIT( u16 ( CPU::*addressingMode )() );
    void ASL( u16 ( CPU::*addressingMode )() );
    void LSR( u16 ( CPU::*addressingMode )() );
    void ROL( u16 ( CPU::*addressingMode )() );
    void ROR( u16 ( CPU::*addressingMode )() );
    void PLA();
    void PLP();
    void AddToReg( u8& reg, u8 value );
    void AddToMemory( u16 ( CPU::*addressingMode )(), u8 value );

    // helpers
    [[nodiscard]] auto GetStatusString() const -> std::string;
    [[nodiscard]] auto Pop() -> u8;
    void               Push( u8 value );
    void               SetZeroAndNegativeFlags( u8 value );
};
