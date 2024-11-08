// cpu.h
#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

using u8 = uint8_t;
using u16 = uint16_t;
using u64 = uint64_t;
using s8 = int8_t;

constexpr size_t size64KB = static_cast<size_t>( 64 * 1024 );
constexpr size_t numOpcodes = 256;
constexpr u16    defaultStartAddress = 0x8000;

// CPU state with optional values to make testing partial states easier.
struct CPUState
{
    std::optional<u16> pc = std::nullopt; // Program Counter
    std::optional<u8>  a = std::nullopt;  // Accumulator
    std::optional<u8>  x = std::nullopt;  // X Register
    std::optional<u8>  y = std::nullopt;  // Y Register
    std::optional<u8>  s = std::nullopt;  // Stack Pointer
    std::optional<u8>  p = std::nullopt;  // Status Register
};

// Forward declaration, to avoid circular dependency
class Bus;

class CPU
{
  public:
    explicit CPU( Bus *bus );

    // Getters and Setters
    [[nodiscard]] auto GetPC() const -> u16;
    [[nodiscard]] auto GetA() const -> u8;
    [[nodiscard]] auto GetX() const -> u8;
    [[nodiscard]] auto GetY() const -> u8;
    [[nodiscard]] auto GetS() const -> u8;
    [[nodiscard]] auto GetP() const -> u8;
    [[nodiscard]] auto IsHalted() const;
    [[nodiscard]] auto GetCycles() const -> u64;

    void SetPC( u16 pcVal );
    void SetA( u8 aVal );
    void SetX( u8 xVal );
    void SetY( u8 yVal );
    void SetS( u8 sVal );
    void SetP( u8 pVal );

    // CPU Methods
    void Reset( CPUState state = {} );
    void FetchDecodeExecute();

    [[nodiscard]] auto Read( u16 address ) const -> u8;
    void               Write( u16 address, u8 data );

    // Helpers
    void LoadProgram( const std::vector<u8> &data, u16 startAddress = defaultStartAddress );
    void PrintMemory( u16 start, u16 end = 0x0000 ) const;
    void PrintRegisters() const;

    // Addressing modes
    auto IMP() -> u16;  // Implied
    auto IMM() -> u16;  // IMM
    auto ZPG() -> u16;  // Zero Page
    auto ABS() -> u16;  // ABS
    auto ABSX() -> u16; // Absolute x
    auto ABSX_NoPageCross() -> u16;
    auto ABSY() -> u16; // Absolute y
    auto ABSY_NoPageCross() -> u16;
    auto ZPGX() -> u16; // Zero Page x
    auto ZPGY() -> u16; // Zero Page y
    auto IND() -> u16;  // Indirect
    auto INDX() -> u16; // Indirect x
    auto INDY() -> u16; // Indirect y
    auto INDY_NoPageCross() -> u16;
    auto REL() -> u16; // Relative

  private:
    // Registers
    u16 _pc;
    u8  _a, _x, _y, _s, _p;
    u64 _cycles;

    // Memory
    std::array<u8, 2048> _ram; // 2KB of internal RAM, with mirroring

    Bus *_bus;

    // Statuses
    enum Status : u8
    {
        Carry = 1 << 0,            // 0b00000001
        Zero = 1 << 1,             // 0b00000010
        InterruptDisable = 1 << 2, // 0b00000100
        Decimal = 1 << 3,          // 0b00001000
        Break = 1 << 4,            // 0b00010000
        Unused = 1 << 5,           // 0b00100000
        Overflow = 1 << 6,         // 0b01000000
        Negative = 1 << 7,         // 0b10000000
    };

    // helper globals
    bool _halt = false;
    bool _supportsDecimalMode = false;

    // Opcodes table
    using OpcodeHandler = void ( * )( CPU & );

    template <void ( CPU::*Op )()> void ExecuteOpcode( CPU &cpu ) { ( cpu.*Op )(); }
    template <void ( CPU::*Op )( void ( CPU::* )() ), void ( CPU::*addressingMode )()>
    void ExecuteOpcode( CPU &cpu )
    {
        ( cpu.*Op )( addressingMode );
    }

    std::array<OpcodeHandler, numOpcodes> _op;

    // Instruction Helpers
    void BRK( u8 cycles );
    void LD( u8 cycles, u16 ( CPU::*addressingMode )(), u8 &reg );
    void ST( u8 cycles, u16 ( CPU::*addressingMode )(), u8 reg );
    void Transfer( u8 cycles, u8 &src, u8 &dest, bool updateFlags = true );
    void AND( u8 cycles, u16 ( CPU::*addressingMode )() );
    void ORA( u8 cycles, u16 ( CPU::*addressingMode )() );
    void EOR( u8 cycles, u16 ( CPU::*addressingMode )() );
    void BIT( u8 cycles, u16 ( CPU::*addressingMode )() );
    void ASL( u8 cycles, u16 ( CPU::*addressingMode )() );
    void LSR( u8 cycles, u16 ( CPU::*addressingMode )() );
    void ROL( u8 cycles, u16 ( CPU::*addressingMode )() );
    void ROR( u8 cycles, u16 ( CPU::*addressingMode )() );
    void PLA( u8 cycles );
    void PLP( u8 cycles );
    void PHP( u8 cycles );
    void PHA( u8 cycles );
    void AddToReg( u8 cycles, u8 &reg, u8 value );
    void AddToMemory( u8 cycles, u16 ( CPU::*addressingMode )(), u8 value );
    void SetFlags( u8 cycles, u8 flag );
    void ClearFlags( u8 cycles, u8 flag );
    void ADC( u8 cycles, u16 ( CPU::*addressingMode )() );
    void SBC( u8 cycles, u16 ( CPU::*addressingMode )() );
    void Compare( u8 cycles, u16 ( CPU::*addressingMode )(), u8 reg );
    void BranchOn( u8 cycles, u8 status, bool condition );
    void JMP( u8 cycles, u16 ( CPU::*addressingMode )() );
    void JSR( u8 cycles );
    void RTS( u8 cycles );
    void RTI( u8 cycles );
    void NOP( u8 cycles );

    // helpers
    [[nodiscard]] auto GetStatusString() const -> std::string;
    [[nodiscard]] auto Pop() -> u8;
    void               Push( u8 value );
    void               SetZeroAndNegativeFlags( u8 value );
};
