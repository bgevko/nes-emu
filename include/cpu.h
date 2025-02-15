#pragma once

#include <array>
#include <cstdint>
#include <string>

// Aliases for integer types
using u8 = uint8_t;
using s8 = int8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using namespace std;

// Forward declaration for reads and writes
class Bus;

class CPU
{
  public:
    explicit CPU( Bus *bus ); // Must pass a pointer to a Bus class on initialization

    /*
    ################################
    ||           Getters          ||
    ################################
    */
    u8   GetAccumulator() const;
    u8   getstuff() const;
    u8   GetXRegister() const;
    u8   GetYRegister() const;
    u8   GetStatusRegister() const;
    u8   GetStackPointer() const;
    u16  GetProgramCounter() const;
    u64  GetCycles() const;
    bool IsReading2002() const { return _reading2002; }
    bool IsNmiInProgress() const { return _nmiInProgress; }

    /*
    ################################
    ||           Setters          ||
    ################################
    */
    void SetAccumulator( u8 value );
    void SetXRegister( u8 value );
    void SetYRegister( u8 value );
    void SetStatusRegister( u8 value );
    void SetStackPointer( u8 value );
    void SetProgramCounter( u16 value );
    void SetCycles( u64 value );
    void SetReading2002( bool value ) { _reading2002 = value; };
    void SetNmiInProgress( bool value ) { _nmiInProgress = value; }

    /*
    ################################
    ||         CPU Methods        ||
    ################################
    */
    u8  Fetch();
    u8  Read( u16 address ) const;
    u8  ReadAndTick( u16 address );
    u8  DummyRead() { return ReadAndTick( _pc ); }                                                 // 1 cycle
    u8  ReadBytePC() { return ReadAndTick( _pc++ ); }                                              // 1 cycle
    u8  ReadByte( u16 address ) { return ReadAndTick( address ); }                                 // 1 cycle
    u16 ReadWordPC() { return ReadBytePC() | ( ReadBytePC() << 8 ); }                              // 2 cycles
    u16 ReadWord( u16 address ) { return ReadByte( address ) | ( ReadByte( address + 1 ) << 8 ); } // 2 cycles
    void Reset();
    void DecodeExecute();
    void Tick();
    void Write( u16 address, u8 data ) const;
    void WriteAndTick( u16 address, u8 data );
    void NMI();
    void IRQ();
    void ExecuteFrame();

    /*
    ################################
    ||        Debug Methods       ||
    ################################
    */
    string LogLineAtPC( bool verbose = true );
    string GetTrace() const { return _trace; }
    void   EnableTracelog() { _traceEnabled = true; }
    void   DisableTracelog() { _traceEnabled = false; }
    void   EnableJsonTestMode() { _isTestMode = true; }
    void   DisableJsonTestMode() { _isTestMode = false; }

  private:
    friend class CPUTestFixture; // Used for testing private methods

    /*
    ################################
    ||          Registers         ||
    ################################
    */
    u16 _pc = 0x0000;
    u8  _a = 0x00;
    u8  _x = 0x00;
    u8  _y = 0x00;
    u8  _s = 0xFD;
    u8  _p = 0x00 | Unused | InterruptDisable;
    u64 _cycles = 0;

    /*
    ################################
    ||      Global Variables      ||
    ################################
    */
    bool   _didVblank = false;
    bool   _isWriteModify = false;
    bool   _currentPageCrossPenalty = true;
    bool   _reading2002 = false;
    bool   _nmiInProgress = false;
    string _instructionName;
    string _addrMode;

    /*
    ################################
    ||       Debug Variables      ||
    ################################
    */
    bool   _isTestMode = false;
    bool   _traceEnabled = false;
    bool   _didTrace = false;
    string _trace;

    /*
    ################################
    ||         Peripherals        ||
    ################################
    */
    Bus *_bus;

    /*
    ################################
    ||        Opcode Table        ||
    ################################
    */
    struct InstructionData {
        string name;                             // Instruction mnemonic (e.g. LDA, STA)
        string addrMode;                         // Addressing mode mnemonic (e.g. ABS, ZPG)
        void ( CPU::*instructionMethod )( u16 ); // Pointer to the instruction helper method
        u16 ( CPU::*addressingModeMethod )();    // Pointer to the address mode helper method
        u8 cycles;                               // Number of cycles the instruction takes
        u8 bytes;                                // Number of bytes the instruction takes
        // Some instructions take an extra cycle if a page boundary is crossed. However, in some
        // cases the extra cycle is not taken if the operation is a read. This will be set
        // selectively for a handful of opcodes, but otherwise will be set to true by default
        bool pageCrossPenalty = true;
        bool isWriteModify = false; // Write/modify instructions use a dummy read before writing,
                                    // spending an extra cycle
    };
    // Opcode table
    array<InstructionData, 256> _opcodeTable;

    /*
    ################################
    ||     Instruction Helpers    ||
    ################################
    */
    // Enum for Status Register
    enum Status : u8 {
        Carry = 1 << 0,            // 0b00000001
        Zero = 1 << 1,             // 0b00000010
        InterruptDisable = 1 << 2, // 0b00000100
        Decimal = 1 << 3,          // 0b00001000
        Break = 1 << 4,            // 0b00010000
        Unused = 1 << 5,           // 0b00100000
        Overflow = 1 << 6,         // 0b01000000
        Negative = 1 << 7,         // 0b10000000
    };

    // Flag methods
    void SetFlags( u8 flag );
    void ClearFlags( u8 flag );
    bool IsFlagSet( u8 flag ) const;
    void SetZeroAndNegativeFlags( u8 value );

    // Load and store helpers
    void LoadRegister( u16 address, u8 &reg );
    void StoreRegister( u16 address, u8 reg );

    // Branch helper
    void BranchOnStatus( u16 offsetAddress, u8 flag, bool isSet );

    // Compare helper
    void CompareAddressWithRegister( u16 address, u8 reg );

    // Push/Pop helper
    void StackPush( u8 value );
    u8   StackPop();

    /*
    ################################
    ||      Addressing Modes      ||
    ################################
    */
    u16 IMP();  // Implicit
    u16 IMM();  // Immediate
    u16 ZPG();  // Zero Page
    u16 ZPGX(); // Zero Page X
    u16 ZPGY(); // Zero Page Y
    u16 ABS();  // Absolute
    u16 ABSX(); // Absolute X
    u16 ABSY(); // Absolute Y
    u16 IND();  // Indirect
    u16 INDX(); // Indirect X
    u16 INDY(); // Indirect Y
    u16 REL();  // Relative

    /*
    ################################
    ||     Instruction Methods    ||
    ################################
    */
    void NOP( u16 address );

    // Load/Store
    void LDA( u16 address );
    void LDX( u16 address );
    void LDY( u16 address );
    void STA( u16 address );
    void STX( u16 address );
    void STY( u16 address );

    // Arithmetic
    void ADC( u16 address );
    void SBC( u16 address );
    void INC( u16 address );
    void DEC( u16 address );
    void INX( u16 address );
    void INY( u16 address );
    void DEX( u16 address );
    void DEY( u16 address );

    // Clear/Set flags
    void CLC( u16 address );
    void CLI( u16 address );
    void CLD( u16 address );
    void CLV( u16 address );
    void SEC( u16 address );
    void SED( u16 address );
    void SEI( u16 address );

    // Branch
    void BPL( u16 address );
    void BMI( u16 address );
    void BVC( u16 address );
    void BVS( u16 address );
    void BCC( u16 address );
    void BCS( u16 address );
    void BNE( u16 address );
    void BEQ( u16 address );

    // Compare
    void CMP( u16 address );
    void CPX( u16 address );
    void CPY( u16 address );

    // Shift
    void ASL( u16 address );
    void LSR( u16 address );
    void ROL( u16 address );
    void ROR( u16 address );

    // Stack
    void PHA( u16 address );
    void PHP( u16 address );
    void PLA( u16 address );
    void PLP( u16 address );
    void TSX( u16 address );
    void TXS( u16 address );

    // Jumps
    void JMP( u16 address );
    void JSR( u16 address );
    void RTS( u16 address );
    void RTI( u16 address );
    void BRK( u16 address );

    // Bitwise
    void AND( u16 address );
    void EOR( u16 address );
    void ORA( u16 address );
    void BIT( u16 address );

    // Transfer
    void TAX( u16 address );
    void TXA( u16 address );
    void TAY( u16 address );
    void TYA( u16 address );

    /*
    ################################
    ||   Unofficial Instructions  ||
    ################################
    */
    void NOP2( u16 address );
    void JAM( u16 address );
    void SLO( u16 address );
    void RLA( u16 address );
    void SRE( u16 address );
    void RRA( u16 address );
    void SAX( u16 address );
    void LAX( u16 address );
    void DCP( u16 address );
    void ISC( u16 address );
    void ALR( u16 address );
    void ARR( u16 address );
    void SHA( u16 address );
    void TAS( u16 address );
    void LXA( u16 address );
    void LAS( u16 address );
    void SBX( u16 address );
    void SHY( u16 address );
    void SHX( u16 address );
    void ANC( u16 address );
};
