// cpu.h
#pragma once
#include <array>
#include <cstdint>
#include <vector>

using _8bit = uint8_t;
using _16bit = uint16_t;

class CPU {
public:
  CPU();
  ~CPU();

  void Reset();
  void Execute();
  void LoadProgram(const std::vector<_8bit> &data, _16bit startAddress);

  _8bit Read(_16bit address);
  int Write(_16bit address, _8bit data);

  void PrintMemory(_16bit start, _16bit end = 0x0000);
  void PrintRegisters();

  // Registers
  _8bit A;      // Accumulator
  _8bit X, Y;   // Index Registers
  _8bit Status; // Status Register
  _16bit ProgramCounter, StackPointer;

  // helper globals
  bool halt = false;

private:
  // Statuses
  enum StatusFlag {
    Carry = 1 << 0,
    Zero = 1 << 1,
    InterruptDisable = 1 << 2,
    Decimal = 1 << 3,
    Break = 1 << 4,
    Unused = 1 << 5,
    Overflow = 1 << 6,
    Negative = 1 << 7,
  };

  // Addressing modes
  _16bit IMM(); // Immediate
  _16bit ZP0(); // Zero Page
  _16bit ABS(); // Absolute
  /* _8bit IMP(); // Implied */
  /* _8bit ZPX(); // Zero Page X */
  /* _8bit ZPY(); // Zero Page Y */
  /* _8bit REL(); // Relative */
  /* _8bit ABX(); // Absolute X */
  /* _8bit ABY(); // Absolute Y */
  /* _8bit IND(); // Indirect */
  /* _8bit IZX(); // Indirect X */
  /* _8bit IZY(); // Indirect Y */

  // Opcodes
  void BRK();
  void LDA(_16bit (CPU::*addressingMode)());
  void STA(_16bit (CPU::*addressingMode)());

  // Status string helper
  std::string GetStatusString();

  // Memory
  std::array<_8bit, 64 * 1024> RAM;
};
