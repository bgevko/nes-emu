// cpu.cpp
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "cpu.h"

CPU::CPU()
{
    Reset();

    // Initialize the opcode table with nullptr
    opcodeTable.fill( nullptr );

// Initialize the opcode table with the appropriate function pointers
#define SET_OP( ... ) []( CPU& cpu ) { cpu.__VA_ARGS__; }  // Macro to simplify table initialization
    opcodeTable[0x00] = SET_OP( BRK() );                   // BRK
    opcodeTable[0x21] = SET_OP( AND( &CPU::INX ) );        // AND (Indirect,X)
    opcodeTable[0xA0] = SET_OP( LD( &CPU::IMM, cpu.y ) );  // LDY (IMM)
    opcodeTable[0xA1] = SET_OP( LD( &CPU::INX, cpu.a ) );  // LDA (X-indirect)
    opcodeTable[0xA2] = SET_OP( LD( &CPU::IMM, cpu.x ) );  // LDX (IMM)
    opcodeTable[0xA4] = SET_OP( LD( &CPU::ZPG, cpu.y ) );  // LDY (ZPG)
    opcodeTable[0xA5] = SET_OP( LD( &CPU::ZPG, cpu.a ) );  // LDA (ZPG)
    opcodeTable[0xA6] = SET_OP( LD( &CPU::ZPG, cpu.x ) );  // LDX (ZPG)
    opcodeTable[0xA9] = SET_OP( LD( &CPU::IMM, cpu.a ) );  // LDA (IMM)
    opcodeTable[0xAC] = SET_OP( LD( &CPU::ABS, cpu.y ) );  // LDY (ABS)
    opcodeTable[0xAD] = SET_OP( LD( &CPU::ABS, cpu.a ) );  // LDA (ABS)
    opcodeTable[0xAE] = SET_OP( LD( &CPU::ABS, cpu.x ) );  // LDX (ABS)
    opcodeTable[0x8D] = SET_OP( STA( &CPU::ABS ) );        // STA (ABS)
}

// ----------------------------------------------------------------------------
// ------------------------------- CPU METHODS --------------------------------
// ----------------------------------------------------------------------------
void CPU::Reset( CPUState state )
{
    // Reset CPU from the provided state or default
    a = state.a.value_or( 0x00 );
    x = state.x.value_or( 0x00 );
    y = state.y.value_or( 0x00 );
    s = state.s.value_or( 0xFD );
    p = state.p.value_or( 0x00 | Unused );

    if ( state.ram.has_value() )
    {
        ram = state.ram.value();
    }
    else
    {
        ram.fill( 0x00 );
    }

    // The program counter is usually read from the reset vector of a game, a 16
    // bit address located at 0xFFFC. If an explicit PC value is not provided, it
    // will be read from ram at 0xFFFC, which is the hardware behavior.
    pc = state.pc.value_or( ( Read( 0xFFFD ) << 8 ) | Read( 0xFFFC ) );
}

u8 CPU::Read( u16 address ) const
{
    if ( !( address >= 0x0000 && address <= 0xFFFF ) )
    {
        throw std::out_of_range( "Read: Invalid read address: " + std::to_string( address ) );
    }
    return ram[address];
}

void CPU::Write( u16 address, u8 data )
{
    if ( !( address >= 0x0000 && address <= 0xFFFF ) )
    {
        throw std::out_of_range( "Write: Invalid write address: " + std::to_string( address ) );
    }
    ram[address] = data;
}

void CPU::FetchDecodeExecute()
{
    // Fetch the opcode from memory
    u8 opcode = Read( pc++ );
    // If the opcode is not implemented, print a warning
    if ( opcodeTable[opcode] == nullptr )
    {
        std::cout << std::endl;
        std::cout << "EXECUTE WARNING, Invalid opcode: " << std::hex << std::setw( 2 )
                  << std::setfill( '0' ) << int( opcode ) << std::endl;
        std::cout << std::endl;
    }
    else
    {
        // Execute the opcode
        opcodeTable[opcode]( *this );
    }
}

// -----------------------------------------------------------------------------
// ------------------------------- ADDRESSING MODES ----------------------------
// -----------------------------------------------------------------------------
u16 CPU::IMM()
{
    // Immediate addressing
    // Returns address of the next byte in memory (the operand itself)
    // The operand is a part of the instruction
    // The program counter is incremented to point to the operand
    return pc++;
}
u16 CPU::ZPG()
{
    // Zero page addressing
    // Returns the address from the zero page (0x0000 - 0x00FF).
    // The value of the next byte (operand) is the address in zero page memory
    return Read( pc++ ) & 0x00FF;
};
u16 CPU::ABS()
{
    // Absolute addressing
    // Constructs a 16 bit address from the next two bytes
    // The first byte (low byte) and the second byte (high byte) form the full
    // address.
    u16 lo = Read( pc++ );
    u16 hi = Read( pc++ );
    return ( hi << 8 ) | lo;
}
u16 CPU::INX()
{
    // Indirect X addressing
    // Reads the next byte, which is a zero page address,
    // and adds the value of the x register to it.
    // The next two bytes form the 16 bit address.
    u8  zpAddress = Read( pc++ );
    u8  offsetAddress = ( zpAddress + x ) & 0x00FF;
    u16 lo = Read( offsetAddress );
    u16 hi = Read( ( offsetAddress + 1 ) & 0x00FF );
    return ( hi << 8 ) | lo;
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
    pc++;  // Padding byte, ignored by the CPU

    // Push program counter to the stack
    Push( pc >> 8 );      // High byte
    Push( pc & 0x00FF );  // Low byte

    // Push the status register to the stack with the break flag
    // B flag and unused flag are pushed to the stack, but ignored when popped.
    u8 status = p | Status::Break | Status::Unused;
    Push( status );

    // Set PC to the address at the interrupt vector (0xFFFE)
    pc = ( Read( 0xFFFF ) << 8 ) | Read( 0xFFFE );

    // Set the interrupt flag
    p |= Status::InterruptDisable;
}

void CPU::AND( u16 ( CPU::*addressingMode )() )
{
    // AND (bitwise AND with accumulator)
    u16 address = ( this->*addressingMode )();
    a &= Read( address );
    SetZeroAndNegativeFlags( a );
};

void CPU::LD( u16 ( CPU::*addressingMode )(), u8& reg )
{
    // Loads a register with a value and sets the zero and negative flags
    u16 address = ( this->*addressingMode )();
    reg = Read( address );
    SetZeroAndNegativeFlags( reg );
};

void CPU::STA( u16 ( CPU::*addressingMode )() )
{
    u16 address = ( this->*addressingMode )();
    Write( address, a );
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
                      << std::setfill( '0' ) << startAddress + i << std::endl;
        }
    }
    pc = startAddress;
}

void CPU::PrintRegisters() const
{
    std::cout << "pc: " << std::hex << std::setw( 4 ) << std::setfill( '0' ) << int( pc )
              << std::endl;
    std::cout << "s: " << std::dec << int( s ) << std::endl;
    std::cout << "a: " << int( a ) << std::endl;
    std::cout << "x: " << int( x ) << std::endl;
    std::cout << "y: " << int( y ) << std::endl;
    std::cout << "p: " << int( p ) << std::endl;
}

void CPU::PrintMemory( u16 start, u16 end ) const
{
    end = end == 0x0000 ? start : end;
    int i = 0;
    while ( start + i <= end )
    {
        std::cout << std::hex << std::setw( 4 ) << std::setfill( '0' ) << start + i << ": "
                  << std::setw( 2 ) << std::setfill( '0' ) << int( Read( start + i ) ) << std::endl;
        i++;
    }
}

std::string CPU::GetStatusString()
{
    // N: Negative, V: Overflow, -: Unused, B: Break, D: Decimal, I: Interrupt
    // Z: Zero, C: Carry
    std::string statusLabel = "NV-BDIZC";
    std::string flags = "";
    for ( int i = 7; i >= 0; i-- )
    {
        flags += ( p & ( 1 << i ) ) ? "1" : "0";
    }
    return statusLabel + " " + flags;
}

void CPU::SetZeroAndNegativeFlags( u8 value )
{
    // Create a mask to clear the Zero and Negative flags (0b01111101)
    p &= ~( Status::Zero | Status::Negative );

    // Set the Zero flag if the value is zero
    if ( value == 0 ) p |= Status::Zero;

    // Set the Negative flag if bit 7 is set
    if ( value & 0x80 )  // Equivalent to (value & 0b10000000)
        p |= Status::Negative;
}

void CPU::Push( u8 value )
{
    // Memory is written to the designated stack addresses, 0x0100 - 0x01FF
    // The stack pointer (sp) points to the next free location on the stack,
    // starting at the last address if the stack is empty (0x01FD)

    // The stack pointer is an 8-bit value, so it wraps around if decremented
    // below 0x0100 (0x00FF -> 0x01FF)
    Write( 0x0100 + s--, value );
}

u8 CPU::Pop()
{
    // The stack pointer is incremented before reading from the stack
    return Read( 0x0100 + ++s );
}
