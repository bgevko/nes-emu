// cpu.cpp
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "cpu.h"

constexpr bool debug = false;

CPU::CPU()  // NOLINT
{
    Reset();

    // Initialize the opcode table with nullptr
    _op.fill( nullptr );

// Initialize the opcode table with the appropriate function pointers
#define SET_OP( ... ) []( CPU& cpu ) { cpu.__VA_ARGS__; }  // NOLINT
    _op[0x00] = SET_OP( BRK() );                           // BRK
    _op[0x01] = SET_OP( ORA( &CPU::INDX ) );               // ORA Indirect X
    _op[0x05] = SET_OP( ORA( &CPU::ZPG ) );                // ORA Zero Page
    _op[0x06] = SET_OP( ASL( &CPU::ZPG ) );                // ASL Zero Page
    _op[0x08] = SET_OP( Push( cpu._p | 0x30 ) );           // PHP (bits 4 and 5 are always set)
    _op[0x09] = SET_OP( ORA( &CPU::IMM ) );                // ORA Immediate
    _op[0x0A] = SET_OP( ASL( &CPU::IMP ) );                // ASL Accumulator
    _op[0x0D] = SET_OP( ORA( &CPU::ABS ) );                // ORA Absolute
    _op[0x0E] = SET_OP( ASL( &CPU::ABS ) );                // ASL Absolute
    _op[0x11] = SET_OP( ORA( &CPU::INDY ) );               // ORA Indirect Y
    _op[0x15] = SET_OP( ORA( &CPU::ZPGX ) );               // ORA Zero Page X
    _op[0x16] = SET_OP( ASL( &CPU::ZPGX ) );               // ASL Zero Page X
    _op[0x18] = SET_OP( ClearFlags( Status::Carry ) );     // CLC (Clear Carry)
    _op[0x19] = SET_OP( ORA( &CPU::ABSY ) );               // ORA Absolute Y
    _op[0x1D] = SET_OP( ORA( &CPU::ABSX ) );               // ORA Absolute X
    _op[0x1E] = SET_OP( ASL( &CPU::ABSX ) );               // ASL Absolute X
    _op[0x21] = SET_OP( AND( &CPU::INDX ) );               // AND Indirect X
    _op[0x11] = SET_OP( ORA( &CPU::INDY ) );               // ORA Indirect Y
    _op[0x21] = SET_OP( AND( &CPU::INDX ) );               // AND Indirect X
    _op[0x24] = SET_OP( BIT( &CPU::ZPG ) );                // BIT Zero Page
    _op[0x25] = SET_OP( AND( &CPU::ZPG ) );                // AND Zero Page
    _op[0x26] = SET_OP( ROL( &CPU::ZPG ) );                // ROL Zero Page
    _op[0x28] = SET_OP( PLP() );                           // PLP (Pull Processor Status)
    _op[0x29] = SET_OP( AND( &CPU::IMM ) );                // AND Immediate
    _op[0x2A] = SET_OP( ROL( &CPU::IMP ) );                // ROL Accumulator
    _op[0x2C] = SET_OP( BIT( &CPU::ABS ) );                // BIT Absolute
    _op[0x2D] = SET_OP( AND( &CPU::ABS ) );                // AND Absolute
    _op[0x2E] = SET_OP( ROL( &CPU::ABS ) );                // ROL Absolute
    _op[0x31] = SET_OP( AND( &CPU::INDY ) );               // AND Indirect Y
    _op[0x35] = SET_OP( AND( &CPU::ZPGX ) );               // AND Zero Page X
    _op[0x36] = SET_OP( ROL( &CPU::ZPGX ) );               // ROL Zero Page X
    _op[0x38] = SET_OP( SetFlags( Status::Carry ) );       // SEC (Set Carry)
    _op[0x39] = SET_OP( AND( &CPU::ABSY ) );               // AND Absolute Y
    _op[0x3D] = SET_OP( AND( &CPU::ABSX ) );               // AND Absolute X
    _op[0x3E] = SET_OP( ROL( &CPU::ABSX ) );               // ROL Absolute X
    _op[0x41] = SET_OP( EOR( &CPU::INDX ) );               // EOR Indirect X
    _op[0x45] = SET_OP( EOR( &CPU::ZPG ) );                // EOR Zero Page
    _op[0x46] = SET_OP( LSR( &CPU::ZPG ) );                // LSR Zero Page
    _op[0x48] = SET_OP( Push( cpu._a ) );                  // PHA
    _op[0x49] = SET_OP( EOR( &CPU::IMM ) );                // EOR Immediate
    _op[0x4A] = SET_OP( LSR( &CPU::IMP ) );                // LSR Accumulator
    _op[0x4D] = SET_OP( EOR( &CPU::ABS ) );                // EOR Absolute
    _op[0x4E] = SET_OP( LSR( &CPU::ABS ) );                // LSR Absolute
    _op[0x51] = SET_OP( EOR( &CPU::INDY ) );               // EOR Indirect Y
    _op[0x55] = SET_OP( EOR( &CPU::ZPGX ) );               // EOR Zero Page X
    _op[0x56] = SET_OP( LSR( &CPU::ZPGX ) );               // LSR Zero Page X
    _op[0x58] = SET_OP( ClearFlags( Status::InterruptDisable ) );  // CLI
    _op[0x59] = SET_OP( EOR( &CPU::ABSY ) );                       // EOR Absolute Y
    _op[0x5D] = SET_OP( EOR( &CPU::ABSX ) );                       // EOR Absolute X
    _op[0x5E] = SET_OP( LSR( &CPU::ABSX ) );                       // LSR Absolute X
    _op[0x66] = SET_OP( ROR( &CPU::ZPG ) );                        // ROR Zero Page
    _op[0x6A] = SET_OP( ROR( &CPU::IMP ) );                        // ROR Accumulator
    _op[0x6E] = SET_OP( ROR( &CPU::ABS ) );                        // ROR Absolute
    _op[0x68] = SET_OP( PLA() );                                   // PLA
    _op[0x76] = SET_OP( ROR( &CPU::ZPGX ) );                       // ROR Zero Page X
    _op[0x78] = SET_OP( SetFlags( Status::InterruptDisable ) );    // SEI (Set Interrupt Disable)
    _op[0x7E] = SET_OP( ROR( &CPU::ABSX ) );                       // ROR Absolute X
    _op[0x81] = SET_OP( ST( &CPU::INDX, cpu._a ) );                // STA Indirect X
    _op[0x84] = SET_OP( ST( &CPU::ZPG, cpu._y ) );                 // STY Zero Page
    _op[0x85] = SET_OP( ST( &CPU::ZPG, cpu._a ) );                 // STA Zero Page
    _op[0x86] = SET_OP( ST( &CPU::ZPG, cpu._x ) );                 // STX Zero Page
    _op[0x88] = SET_OP( AddToReg( cpu._y, -1 ) );                  // DEY
    _op[0x8A] = SET_OP( Transfer( cpu._x, cpu._a ) );              // TXA
    _op[0x8C] = SET_OP( ST( &CPU::ABS, cpu._y ) );                 // STY Absolute
    _op[0x8D] = SET_OP( ST( &CPU::ABS, cpu._a ) );                 // STA Absolute
    _op[0x8E] = SET_OP( ST( &CPU::ABS, cpu._x ) );                 // STX Absolute
    _op[0x91] = SET_OP( ST( &CPU::INDY, cpu._a ) );                // STA Indirect Y
    _op[0x94] = SET_OP( ST( &CPU::ZPGX, cpu._y ) );                // STY Zero Page X
    _op[0x95] = SET_OP( ST( &CPU::ZPGX, cpu._a ) );                // STA Zero Page X
    _op[0x96] = SET_OP( ST( &CPU::ZPGY, cpu._x ) );                // STX Zero Page Y
    _op[0x98] = SET_OP( Transfer( cpu._y, cpu._a ) );              // TYA
    _op[0x99] = SET_OP( ST( &CPU::ABSY, cpu._a ) );                // STA Absolute Y
    _op[0x9A] = SET_OP( Transfer( cpu._x, cpu._s, false ) );       // TXS
    _op[0x9D] = SET_OP( ST( &CPU::ABSX, cpu._a ) );                // STA Absolute X
    _op[0xA0] = SET_OP( LD( &CPU::IMM, cpu._y ) );                 // LDY Immediate
    _op[0xA1] = SET_OP( LD( &CPU::INDX, cpu._a ) );                // LDA Indirect X
    _op[0xA2] = SET_OP( LD( &CPU::IMM, cpu._x ) );                 // LDX Immediate
    _op[0xA4] = SET_OP( LD( &CPU::ZPG, cpu._y ) );                 // LDY Zero Page
    _op[0xA5] = SET_OP( LD( &CPU::ZPG, cpu._a ) );                 // LDA Zero Page
    _op[0xA6] = SET_OP( LD( &CPU::ZPG, cpu._x ) );                 // LDX Zero Page
    _op[0xA8] = SET_OP( Transfer( cpu._a, cpu._y ) );              // TAY
    _op[0xA9] = SET_OP( LD( &CPU::IMM, cpu._a ) );                 // LDA Immediate
    _op[0xAA] = SET_OP( Transfer( cpu._a, cpu._x ) );              // TAX
    _op[0xAC] = SET_OP( LD( &CPU::ABS, cpu._y ) );                 // LDY Absolute
    _op[0xAD] = SET_OP( LD( &CPU::ABS, cpu._a ) );                 // LDA Absolute
    _op[0xAE] = SET_OP( LD( &CPU::ABS, cpu._x ) );                 // LDX Absolute
    _op[0xB1] = SET_OP( LD( &CPU::INDY, cpu._a ) );                // LDA Indirect Y
    _op[0xB4] = SET_OP( LD( &CPU::ZPGX, cpu._y ) );                // LDY Zero Page X
    _op[0xB5] = SET_OP( LD( &CPU::ZPGX, cpu._a ) );                // LDA Zero Page X
    _op[0xB6] = SET_OP( LD( &CPU::ZPGY, cpu._x ) );                // LDX Zero Page Y
    _op[0xB8] = SET_OP( ClearFlags( Status::Overflow ) );          // CLV
    _op[0xB9] = SET_OP( LD( &CPU::ABSY, cpu._a ) );                // LDA Absolute Y
    _op[0xBA] = SET_OP( Transfer( cpu._s, cpu._x ) );              // TSX
    _op[0xBC] = SET_OP( LD( &CPU::ABSX, cpu._y ) );                // LDY Absolute X
    _op[0xBD] = SET_OP( LD( &CPU::ABSX, cpu._a ) );                // LDA Absolute X
    _op[0xBE] = SET_OP( LD( &CPU::ABSY, cpu._x ) );                // LDX Absolute Y
    _op[0xCA] = SET_OP( AddToReg( cpu._x, -1 ) );                  // DEX
    _op[0xC6] = SET_OP( AddToMemory( &CPU::ZPG, -1 ) );            // DEC Zero Page
    _op[0xC8] = SET_OP( AddToReg( cpu._y, 1 ) );                   // INY
    _op[0xCE] = SET_OP( AddToMemory( &CPU::ABS, -1 ) );            // DEC Absolute
    _op[0xD6] = SET_OP( AddToMemory( &CPU::ZPGX, -1 ) );           // DEC Zero Page X
    _op[0xD8] = SET_OP( ClearFlags( Status::Decimal ) );           // CLD
    _op[0xDE] = SET_OP( AddToMemory( &CPU::ABSX, -1 ) );           // DEC Absolute X
    _op[0xE6] = SET_OP( AddToMemory( &CPU::ZPG, 1 ) );             // INC Zero Page
    _op[0xE8] = SET_OP( AddToReg( cpu._x, 1 ) );                   // INX
    _op[0xEE] = SET_OP( AddToMemory( &CPU::ABS, 1 ) );             // INC Absolute
    _op[0xF6] = SET_OP( AddToMemory( &CPU::ZPGX, 1 ) );            // INC Zero Page X
    _op[0xF8] = SET_OP( SetFlags( Status::Decimal ) );             // SED (Set Decimal Flag)
    _op[0xFE] = SET_OP( AddToMemory( &CPU::ABSX, 1 ) );            // INC Absolute X
}

// ----------------------------------------------------------------------------
// ------------------------------- GETTERS ------------------------------------
// ----------------------------------------------------------------------------
[[nodiscard]] auto CPU::GetA() const -> u8 { return _a; }
[[nodiscard]] auto CPU::GetX() const -> u8 { return _x; }
[[nodiscard]] auto CPU::GetY() const -> u8 { return _y; }
[[nodiscard]] auto CPU::GetS() const -> u8 { return _s; }
[[nodiscard]] auto CPU::GetP() const -> u8 { return _p; }
[[nodiscard]] auto CPU::GetPC() const -> u16 { return _pc; }
[[nodiscard]] auto CPU::GetMemory() const -> const std::array<u8, kMemorySize>& { return _memory; }
[[nodiscard]] auto CPU::IsHalted() const { return _halt; }

// ----------------------------------------------------------------------------
// ------------------------------- SETTERS ------------------------------------
// ----------------------------------------------------------------------------
void CPU::SetA( u8 aVal ) { _a = aVal; }
void CPU::SetX( u8 xVal ) { _x = xVal; }
void CPU::SetY( u8 yVal ) { _y = yVal; }
void CPU::SetS( u8 sVal ) { _s = sVal; }
void CPU::SetP( u8 pVal ) { _p = pVal; }
void CPU::SetPC( u16 pcVal ) { _pc = pcVal; }
void CPU::SetMemory( const std::array<u8, kMemorySize>& memory ) { _memory = memory; }
void CPU::SetHalted( bool halt ) { _halt = halt; }

// ----------------------------------------------------------------------------
// ------------------------------- CPU METHODS --------------------------------
// ----------------------------------------------------------------------------
void CPU::Reset( CPUState state )
{
    // Reset CPU from the provided state or default
    _a = state.a.value_or( 0x00 );
    _x = state.x.value_or( 0x00 );
    _y = state.y.value_or( 0x00 );
    _s = state.s.value_or( 0xFD );
    _p = state.p.value_or( 0x00 | Unused );

    if ( state.memory.has_value() )
    {
        _memory = state.memory.value();
    }
    else
    {
        _memory.fill( 0x00 );
    }

    // The program counter is usually read from the reset vector of a game, a 16
    // bit address located at 0xFFFC. If an explicit PC value is not provided, it
    // will be read from ram at 0xFFFC, which is the hardware behavior.
    _pc = state.pc.value_or( ( Read( 0xFFFD ) << 8 ) | Read( 0xFFFC ) );
}

auto CPU::Read( u16 address ) const -> u8
{
    if ( address < 0x0000 || address > 0xFFFF )
    {
        throw std::out_of_range( "Read: Invalid read address: " + std::to_string( address ) );
    }
    return _memory[address];
}

void CPU::Write( u16 address, u8 data )
{
    if ( address < 0x0000 || address > 0xFFFF )
    {
        throw std::out_of_range( "Write: Invalid write address: " + std::to_string( address ) );
    }
    _memory[address] = data;
}

void CPU::FetchDecodeExecute()
{
    // Fetch the opcode from memory
    u8 opcode = Read( _pc++ );
    // If the opcode is not implemented, print a warning
    if ( _op[opcode] == nullptr )
    {
        std::cout << '\n';
        std::cout << "EXECUTE WARNING, Invalid opcode: " << std::hex << std::setw( 2 )
                  << std::setfill( '0' ) << int( opcode ) << '\n';
        std::cout << '\n';
    }
    else
    {
        // Execute the opcode
        _op[opcode]( *this );
    }
}

// -----------------------------------------------------------------------------
// ------------------------------- ADDRESSING MODES ----------------------------
// -----------------------------------------------------------------------------

auto CPU::IMP() -> u16
{
    // Implied addressing
    // No operand is required for the instruction
    // The instruction operates on the accumulator or the status register
    // The program counter is not incremented
    return 0;
}

auto CPU::IMM() -> u16
{
    // Immediate addressing
    // Returns address of the next byte in memory (the operand itself)
    // The operand is a part of the instruction
    // The program counter is incremented to point to the operand
    return _pc++;
}
auto CPU::ZPG() -> u16
{
    // Zero page addressing
    // Returns the address from the zero page (0x0000 - 0x00FF).
    // The value of the next byte (operand) is the address in zero page memory
    return Read( _pc++ ) & 0x00FF;
};
auto CPU::ZPGX() -> u16
{
    // Zero page addressing with X offset
    // Returns the address from the zero page (0x0000 - 0x00FF) with the X register offset
    // The value of the next byte (operand) is the address in zero page memory
    return ( Read( _pc++ ) + _x ) & 0x00FF;
}
auto CPU::ZPGY() -> u16
{
    // Zero page addressing with Y offset
    return ( Read( _pc++ ) + _y ) & 0x00FF;
}
auto CPU::ABS() -> u16
{
    // Absolute addressing
    // Constructs a 16 bit address from the next two bytes
    // The first byte (low byte) and the second byte (high byte) form the full
    // address.
    u16 low = Read( _pc++ );
    u16 high = Read( _pc++ );
    return ( high << 8 ) | low;
}

auto CPU::ABSX() -> u16
{
    // Absolute addressing with X offset
    // Constructs a 16 bit address from the next two bytes
    // Adds the value of the X register to the address
    u16 low = Read( _pc++ );
    u16 high = Read( _pc++ );
    u16 address = ( high << 8 ) | low;
    u16 finalAddress = address + _x;

    // NOTE: Boundary crossing will add an extra cycle (Not implemented yet)
    return finalAddress;
}

auto CPU::ABSY() -> u16
{
    // Absolute addressing with Y offset
    // Constructs a 16 bit address from the next two bytes
    // Adds the value of the Y register to the address
    u16 low = Read( _pc++ );
    u16 high = Read( _pc++ );
    u16 address = ( high << 8 ) | low;
    u16 finalAddress = address + _y;
    // NOTE: Boundary crossing will add an extra cycle (Not implemented yet)
    return finalAddress;
}

auto CPU::IND() -> u16
{
    // Read two bytes to form the pointer address
    u16 ptrLow = Read( _pc++ );
    u16 ptrHigh = Read( _pc++ );
    u16 ptr = ( ptrHigh << 8 ) | ptrLow;

    u8 effLow = Read( ptr );
    u8 effHigh;  // NOLINT

    // 6502 Bug: If the pointer address wraps around a page boundary (e.g. 0x01FF),
    // the CPU reads the low byte from 0x01FF and the high byte from the start of
    // the same page (0x0100) instead of the start of the next page (0x0200).
    if ( ptrLow == 0xFF )
    {
        effHigh = Read( ptr & 0xFF00 );
    }
    else
    {
        effHigh = Read( ptr + 1 );
    }

    return ( effHigh << 8 ) | effLow;
}

auto CPU::INDX() -> u16
{
    // Indirect X addressing
    // Takes the next two bytes as a zero page address
    // Adds the value of the X register to get the pointer address
    // Reads the effective address from the pointer address
    u8  zeroPageAddr = Read( _pc++ );
    u8  offsetAddr = ( zeroPageAddr + _x ) & 0x00FF;
    u16 low = Read( offsetAddr );
    u16 high = Read( ( offsetAddr + 1 ) & 0x00FF );
    return ( high << 8 ) | low;
}

auto CPU::INDY() -> u16
{
    // Indirect Y addressing
    // Fetches the pointer address in the zero page
    // Reads the effective address from the pointer address
    // Adds the value of the Y register to the effective address
    u8  zeroPageAddr = Read( _pc++ );
    u16 low = Read( zeroPageAddr );
    u16 high = Read( ( zeroPageAddr + 1 ) & 0x00FF );

    u16 effAddress = ( high << 8 ) | low;
    u16 finalAddress = effAddress + _y;

    // NOTE: Boundary crossing will add an extra cycle (Not implemented yet)
    return finalAddress;
}

auto CPU::REL() -> u16
{
    // Relative addressing
    // The next byte is a signed offset from the current program counter
    // The offset is between -128 and 127 bytes
    s8  offset = static_cast<s8>( Read( _pc ) );
    u16 relativeAddr = _pc + offset;
    _pc++;
    return relativeAddr;
}

// ----------------------------------------------------------------------------
// -------------------------------- OPCODES -----------------------------------
// ----------------------------------------------------------------------------
void CPU::BRK()
{
    /*
      BRK
      Force Break

      BRK initiates a software interrupt similar to a hardware
      interrupt (IRQ). The return address pushed to the stack is
      PC+2, providing an extra byte of spacing for a break mark
      (identifying a reason for the break.)
      The status register will be pushed to the stack with the break
      flag set to 1. However, when retrieved during RTI or by a PLP
      instruction, the break flag will be ignored.
      The interrupt disable flag is not set automatically.

      interrupt,
      push PC+2, push SR
      N	Z	C	I	D	V
      -	-	-	1	-	-
      addressing	assembler	opc	bytes	cycles
      implied	BRK	00	1	7
     */
    _pc++;  // Padding byte, ignored by the CPU

    // Push program counter to the stack
    Push( _pc >> 8 );      // High byte
    Push( _pc & 0x00FF );  // Low byte

    // Push the status register to the stack with the break flag
    // B flag and unused flag are pushed to the stack, but ignored when popped.
    u8 status = _p | Status::Break | Status::Unused;
    Push( status );

    // Set PC to the address at the interrupt vector (0xFFFE)
    _pc = ( Read( 0xFFFF ) << 8 ) | Read( 0xFFFE );

    // Set the interrupt flag
    _p |= Status::InterruptDisable;
}

void CPU::LD( u16 ( CPU::*addressingMode )(), u8& reg )
{
    /*
      LD is used by all loading instructions (LDA, LDX, LDY)
      It loads a register with a value from memory, dictated by the addressing mode
      and sets the zero and negative flags based on the value loaded.
    */

    // Loads a register with a value and sets the zero and negative flags
    u16 address = ( this->*addressingMode )();
    u8  value = Read( address );
    reg = value;
    SetZeroAndNegativeFlags( reg );

    if ( debug )
    {
        std::cout << std::hex << std::setfill( '0' );
        std::cout << "LD Instruction Debug:\n";
        std::cout << "Address: 0x" << std::setw( 4 ) << address << "\n";
        std::cout << "Value:   0x" << std::setw( 2 ) << int( value ) << "\n";
        std::cout << "Reg:     0x" << std::setw( 2 ) << int( reg ) << "\n";
        std::cout << std::dec;
        std::cout << "\n";
    }
};

void CPU::ST( u16 ( CPU::*addressingMode )(), u8 reg )
{
    /*
    ST is used by all storing instructions (STA, STX, STY)
    It stores the value of a register into memory, dictated by the addressing mode.
    */

    // Get the address from the specified addressing mode
    u16 address = ( this->*addressingMode )();

    // Store the register value into memory at the address
    Write( address, reg );

    if ( debug )
    {
        std::cout << std::hex << std::setfill( '0' );
        std::cout << "ST Instruction Debug:\n";
        std::cout << "Address: 0x" << std::setw( 4 ) << address << "\n";
        std::cout << "Reg:     0x" << std::setw( 2 ) << int( reg ) << "\n";
        std::cout << std::dec;
        std::cout << "\n";
    }
}

void CPU::AND( u16 ( CPU::*addressingMode )() )
{
    /*
    AND (bitwise AND with accumulator)
    Computes the bitwise AND of the accumulator with a value in memory, dictated by the addressing
    mode. Sets the zero and negative flags based on the result.
     */

    // AND (bitwise AND with accumulator)
    u16 address = ( this->*addressingMode )();
    _a &= Read( address );
    SetZeroAndNegativeFlags( _a );
};

void CPU::ORA( u16 ( CPU::*addressingMode )() )
{
    /*
      ORA (bitwise OR with accumulator)
          Computes the bitwise OR of the accumulator with a value in memory, dictated by the
      addressing mode. Sets the zero and negative flags based on the result.
    */

    u16 address = ( this->*addressingMode )();
    u8  value = Read( address );
    _a |= value;

    SetZeroAndNegativeFlags( _a );

    if ( debug )
    {
        std::cout << std::hex << std::setfill( '0' );
        std::cout << "ORA Instruction Debug:\n";
        std::cout << "Address: 0x" << std::setw( 4 ) << address << "\n";
        std::cout << "Value:   0x" << std::setw( 2 ) << int( value ) << "\n";
        std::cout << "Reg:     0x" << std::setw( 2 ) << int( _a ) << "\n";
        std::cout << std::dec;
        std::cout << "\n";
    }
}

void CPU::EOR( u16 ( CPU::*addressingMode )() )
{
    /*
      EOR (bitwise exclusive OR with accumulator)
      Computes the bitwise exclusive OR of the accumulator with a value in memory, dictated by the
      addressing mode. Sets the zero and negative flags based on the result.
    */

    u16 address = ( this->*addressingMode )();
    u8  value = Read( address );
    _a ^= value;
    SetZeroAndNegativeFlags( _a );
}

void CPU::BIT( u16 ( CPU::*addressingMode )() )
{
    /*
      BIT (bit test)
      Performs an AND operation between the accumulator and a memory location and updates the status
      flags accordingly. The actual result is not stored anywhere.
    */
    u16 address = ( this->*addressingMode )();
    u8  value = Read( address );

    // Set the zero and negative flags based on the result
    SetZeroAndNegativeFlags( _a & value );

    // Set overflow flag to bit 6 of the memory value
    u8 sixBit = ( value & 0x40 ) >> 6;
    _p = sixBit == 1 ? _p | Status::Overflow : _p & ~Status::Overflow;

    // Set the negative flag to bit 7 of the memory value
    u8 sevenBit = ( value & 0x80 ) >> 7;
    _p = sevenBit == 1 ? _p | Status::Negative : _p & ~Status::Negative;
}

void CPU::Transfer( u8& src, u8& dest, bool updateFlags )
{
    /* Transfer is used by the transfer instructions (TAX, TAY, TSX, TXA, TXS, TYA) and
      transfers a value from one register to another. The zero and negative flags are set based on
      the destination register.
    */

    dest = src;
    if ( updateFlags )
    {
        SetZeroAndNegativeFlags( dest );
    }
}

void CPU::ASL( u16 ( CPU::*addressingMode )() )
{
    /* Arithmetic Shift Left
     Shifts all the bits of the accumulator (if implied addressing) or memory one bit left.
     Bit 0 is set to 0 and big 7 is placed to the carry flag. This effect is similar to
     multiplying memory contents by 2.
    */

    // Grab address and value from register A if implied addressing mode, from memory otherwise
    u16 address = addressingMode == &CPU::IMP ? 0 : ( this->*addressingMode )();
    u8  value = addressingMode == &CPU::IMP ? _a : Read( address );

    // Extract the msb to check if it's set
    u8 msb = ( value & 0x80 ) >> 7;

    // If MSB is set, then it will carry over to the carry flag
    _p = msb == 1 ? _p | Status::Carry : _p & ~Status::Carry;

    // Shift and set the zero and negative flags
    SetZeroAndNegativeFlags( value <<= 1 );

    // Write the result back to memory or the accumulator
    if ( addressingMode == &CPU::IMP )
    {
        _a = value;
    }
    else
    {
        Write( address, value );
    }
}

void CPU::LSR( u16 ( CPU::*addressingMode )() )
{
    /*
     Logical Shift Right
    Each bit in the accumulator (if implied addressing) or memory is shifted one bit to the right.
    If there was a bit in the 0 (LSB) position, it is shifted into the carry. Bit 7 is set to 0.
    */
    u16 address = addressingMode == &CPU::IMP ? 0 : ( this->*addressingMode )();
    u8  value = addressingMode == &CPU::IMP ? _a : Read( address );

    // Extract the lsb to check if it's set
    u8 lsb = value & 0x01;

    // If LSB is set, then it will carry over to the carry flag
    _p = lsb == 1 ? _p | Status::Carry : _p & ~Status::Carry;

    // Shift and set the zero and negative flags
    SetZeroAndNegativeFlags( value >>= 1 );

    // Write the result back to memory or the accumulator
    if ( addressingMode == &CPU::IMP )
    {
        _a = value;
    }
    else
    {
        Write( address, value );
    }
}

void CPU::ROL( u16 ( CPU::*addressingMode )() )
{
    /*
     Rotate Left
    Each bit in the accumulator (if implied addressing) or memory is rotated one bit to the left.
    The carry is shifted into bit 0 and bit 7 is shifted into the carry.
    */
    u16 address = addressingMode == &CPU::IMP ? 0 : ( this->*addressingMode )();
    u8  value = addressingMode == &CPU::IMP ? _a : Read( address );

    // Extract the msb to check if it's set
    u8 msb = ( value & 0x80 ) >> 7;

    // Shift left and set the 0th bit if the carry flag is set
    value <<= 1;
    if ( ( _p & Status::Carry ) != 0 )
    {
        value |= 0x01;
    }

    // Set the carry flag if the msb is set
    _p = msb == 1 ? _p | Status::Carry : _p & ~Status::Carry;

    // Set the zero and negative flags
    SetZeroAndNegativeFlags( value );

    // Write the result back to memory or the accumulator
    if ( addressingMode == &CPU::IMP )
    {
        _a = value;
    }
    else
    {
        Write( address, value );
    }
}

void CPU::ROR( u16 ( CPU::*addressingMode )() )
{
    /*
     Rotate Right
    Each bit in the accumulator (if implied addressing) or memory is rotated one bit to the right.
    The carry is shifted into bit 7 and bit 0 is shifted into the carry.
    */
    u16 address = addressingMode == &CPU::IMP ? 0 : ( this->*addressingMode )();
    u8  value = addressingMode == &CPU::IMP ? _a : Read( address );

    // Extract the lsb to check if it's set
    u8 lsb = value & 0x01;

    // Shift right and set the 7th bit if the carry flag is set
    value >>= 1;
    if ( ( _p & Status::Carry ) != 0 )
    {
        value |= 0x80;
    }

    // Set the carry flag if the lsb is set
    _p = lsb == 1 ? _p | Status::Carry : _p & ~Status::Carry;

    // Set the zero and negative flags
    SetZeroAndNegativeFlags( value );

    // Write the result back to memory or the accumulator
    if ( addressingMode == &CPU::IMP )
    {
        _a = value;
    }
    else
    {
        Write( address, value );
    }
}

void CPU::PLA()
{
    /* Pull Accumulator
      Pulls value from the stack into the accumulator, sets the zero and negative flags based on the
    */
    _a = Pop();
    SetZeroAndNegativeFlags( _a );
}

void CPU::PLP()
{
    /* Pull Processor Status
      Pulls value from the stack into the status register, ignores the break flag and sets the
      unused flag.
    */
    u8 value = Pop();

    // Ignore the Break flag (bit 4) and ensure the Unused flag (bit 5) is set
    _p = ( value & 0xEF ) | 0x20;
}

void CPU::AddToReg( u8& reg, u8 value )
{
    /* Modify Register
      Used by the INC and DEC instructions to increment or decrement a register.
    */
    reg += value;
    SetZeroAndNegativeFlags( reg );
}

void CPU::AddToMemory( u16 ( CPU::*addressingMode )(), u8 value )
{
    /* Modify Memory
      Used by the INC and DEC instructions to increment or decrement a memory location.
    */
    u16 address = ( this->*addressingMode )();
    u8  memValue = Read( address );
    memValue += value;
    SetZeroAndNegativeFlags( memValue );
    Write( address, memValue );
}

void CPU::SetFlags( u8 flag )
{
    /* Set Flag
      Used by the SEC, SED, and SEI instructions to set a flag in the status register.
    */
    _p |= flag;
}
void CPU::ClearFlags( u8 flag )
{
    /* Clear Flag
      Used by the CLC, CLD, and CLI instructions to clear a flag in the status register.
    */
    _p &= ~flag;
}

// ----------------------------------------------------------------------------
// ------------------------------- HELPERS ------------------------------------
// ----------------------------------------------------------------------------

void CPU::LoadProgram( const std::vector<u8>& data, u16 startAddress )
{
    for ( size_t i = 0; i < data.size(); i++ )
    {
        try
        {
            Write( startAddress + i, data[i] );
        }
        catch ( const std::out_of_range& e )
        {
            std::cerr << "LoadProgram: Failed to write to address: " << std::hex << std::setw( 4 )
                      << std::setfill( '0' ) << startAddress + i << '\n';
        }
    }
    _pc = startAddress;
}

void CPU::PrintRegisters() const
{
    std::cout << "pc: " << std::hex << std::setw( 4 ) << std::setfill( '0' ) << int( _pc ) << '\n';
    std::cout << "s: " << std::dec << int( _s ) << '\n';
    std::cout << "a: " << int( _a ) << '\n';
    std::cout << "x: " << int( _x ) << '\n';
    std::cout << "y: " << int( _y ) << '\n';
    std::cout << "p: " << int( _p ) << '\n';
}

void CPU::PrintMemory( u16 start, u16 end ) const
{
    end = end == 0x0000 ? start : end;
    int i = 0;  // NOLINT
    while ( start + i <= end )
    {
        std::cout << std::hex << std::setw( 4 ) << std::setfill( '0' ) << start + i << ": "
                  << std::setw( 2 ) << std::setfill( '0' ) << int( Read( start + i ) ) << '\n';
        i++;
    }
}

auto CPU::GetStatusString() const -> std::string
{
    // N: Negative, V: Overflow, -: Unused, B: Break, D: Decimal, I: Interrupt
    // Z: Zero, C: Carry
    std::string statusLabel = "NV-BDIZC";
    std::string flags;
    for ( int i = 7; i >= 0; i-- )
    {
        flags += ( _p & ( 1 << i ) ) != 0 ? "1" : "0";
    }
    return statusLabel + " " + flags;
}

void CPU::SetZeroAndNegativeFlags( u8 value )
{
    // Create a mask to clear the Zero and Negative flags (0b01111101)
    _p &= ~( Status::Zero | Status::Negative );

    // Set the Zero flag if the value is zero
    if ( value == 0 )
    {
        _p |= Status::Zero;
    }

    // Set the Negative flag if bit 7 is set
    if ( ( value & 0x80 ) != 0 )
    {  // Equivalent to (value & 0b10000000)
        _p |= Status::Negative;
    }
}

void CPU::Push( u8 value )
{
    // Memory is written to the designated stack addresses, 0x0100 - 0x01FF
    // The stack pointer (sp) points to the next free location on the stack,
    // starting at the last address if the stack is empty (0x01FD)

    // The stack pointer is an 8-bit value, so it wraps around if decremented
    // below 0x0100 (0x00FF -> 0x01FF)
    Write( 0x0100 + _s--, value );
}

auto CPU::Pop() -> u8
{
    // The stack pointer is incremented before reading from the stack
    return Read( 0x0100 + ++_s );
}
