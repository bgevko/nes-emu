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

  // Memory
  std::array<u8, 64 * 1024> ram;

  // Registers
  u16 pc;
  u8 a, x, y, s, p;

  // CPU Methods
  void Reset(CPUState state = {});
  void FetchDecodeExecute();

  u8 Read(u16 address) const;
  void Write(u16 address, u8 data);

  // Helpers
  void LoadProgram(const std::vector<u8> &data, u16 startAddress = 0x8000);
  void PrintMemory(u16 start, u16 end = 0x0000) const;
  void PrintRegisters() const;

  // helper globals
  bool halt = false;

private:
  // Statuses
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

  // helpers
  std::string GetStatusString();
  void SetZeroAndNegativeFlags(u8 value);
  void Push(u8 value);
  u8 Pop();
};
