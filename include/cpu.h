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
  void LoadProgram(const std::vector<_8bit> &data, _16bit startAddress);

  _8bit Read(_16bit address);
  int Write(_16bit address, _8bit data);

  void PrintMemory(_16bit start, _16bit end);
  void PrintRegisters();

private:
  // Registers
  _8bit A;      // Accumulator
  _8bit X, Y;   // Index Registers
  _8bit Status; // Status Register
  _16bit ProgramCounter, StackPointer;

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

  // Status string helper
  std::string GetStatusString();

  // Memory
  std::array<_8bit, 64 * 1024> RAM;
};
