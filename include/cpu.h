// cpu.h
#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <vector>

using u8 = uint8_t;
using u16 = uint16_t;

// CPU state with optional values to make testing partial states easier.
struct CPUState {
  std::optional<u16> pc = std::nullopt; // Program Counter
  std::optional<u8> a = std::nullopt;   // Accumulator
  std::optional<u8> x = std::nullopt;   // X Register
  std::optional<u8> y = std::nullopt;   // Y Register
  std::optional<u8> s = std::nullopt;   // Stack Pointer
  std::optional<u8> p = std::nullopt;   // Status Register
  std::optional<std::array<u8, 64 * 1024>> ram = std::nullopt; // Memory
};

class CPU {
public:
  CPU();
  ~CPU();

  // Memory
  std::array<u8, 64 * 1024> ram;

  // Registers
  u16 pc;
  u8 a, x, y, s, p;

  // Methods
  void Reset(CPUState state = {});
  void Execute();
  void LoadProgram(const std::vector<u8> &data, u16 startAddress = 0x8000);

  u8 Read(u16 address) const;
  int Write(u16 address, u8 data);

  void PrintMemory(u16 start, u16 end = 0x0000) const;
  void PrintRegisters() const;

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
  u16 Immediate(); // Immediate
  u16 ZeroPage();  // Zero Page
  u16 Absolute();  // Absolute
  /* u8 IMP(); // Implied */
  /* u8 ZPX(); // Zero Page x */
  /* u8 ZPY(); // Zero Page y */
  /* u8 REL(); // Relative */
  /* u8 ABX(); // Absolute x */
  /* u8 ABY(); // Absolute y */
  /* u8 IND(); // Indirect */
  u16 IndirectX(); // Indirect x
  /* u8 IZY(); // Indirect y */

  // Opcodes
  void BRK();
  void LDA(u16 (CPU::*addressingMode)());
  void STA(u16 (CPU::*addressingMode)());
  void AND(u16 (CPU::*addressingMode)());

  // status string helper
  std::string GetStatusString();
};
