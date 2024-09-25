// cpu.cpp

#include "cpu.h"
#include <iomanip>
#include <iostream>

CPU::CPU() { Reset(); }
CPU::~CPU() {}

void CPU::LoadProgram(const std::vector<u8> &data, u16 startAddress) {
  for (size_t i = 0; i < data.size(); i++) {
    if (Write(startAddress + i, data[i]) == -1) {
      std::cerr << "LoadProgram: Failed to write to address: " << std::hex
                << std::setw(4) << std::setfill('0') << startAddress + i
                << std::endl;
    }
  }
  pc = startAddress;
}

void CPU::Reset(CPUState state) {
  // Reset CPU state from the provided argument, or to default of state isn't
  // provided.
  a = state.a.value_or(0x00);
  x = state.x.value_or(0x00);
  y = state.y.value_or(0x00);
  s = state.s.value_or(0xFD);
  p = state.p.has_value() ? state.p.value() : 0x00 | Unused;

  if (state.ram.has_value()) {
    ram = state.ram.value();
  } else {
    ram.fill(0x00);
  }

  // The program counter is usually read from the reset vector of a game, a 16
  // bit address located at 0xFFFC. If an explicit PC value is not provided, it
  // will be read from ram at 0xFFFC, which is the hardware behavior.
  pc = state.pc.value_or((Read(0xFFFD) << 8) | Read(0xFFFC));
}

u8 CPU::Read(u16 address) const {
  if (address >= 0x0000 && address <= 0xFFFF) {
    return ram[address];
  }
  return -1;
}

int CPU::Write(u16 address, u8 data) {
  if (address >= 0x0000 && address <= 0xFFFF) {
    ram[address] = data;
    return 0;
  }
  return -1;
}

void CPU::PrintMemory(u16 start, u16 end) const {
  end = end == 0x0000 ? start : end;
  int i = 0;
  while (start + i <= end) {
    std::cout << std::hex << std::setw(4) << std::setfill('0') << start + i
              << ": " << std::setw(2) << std::setfill('0')
              << int(Read(start + i)) << std::endl;
    i++;
  }
}

std::string CPU::GetStatusString() {
  std::string statusLabel = "NV-BDIZC";
  std::string flags = "";
  for (int i = 7; i >= 0; i--) {
    flags += (p & (1 << i)) ? "1" : "0";
  }
  return statusLabel + " " + flags;
}

void CPU::PrintRegisters() const {
  std::cout << "pc: " << std::hex << std::setw(4) << std::setfill('0')
            << int(pc) << std::endl;
  std::cout << "s: " << std::dec << int(s) << std::endl;
  std::cout << "a: " << int(a) << std::endl;
  std::cout << "x: " << int(x) << std::endl;
  std::cout << "y: " << int(y) << std::endl;
  std::cout << "p: " << int(p) << std::endl;
  /* std::cout << "p: " << GetStatusString() << std::endl; */
}

// Addressing Modes
u16 CPU::Immediate() {
  // Immediate addressing
  // Returns address of the next byte in memory (the operand itself)
  // The operand is a part of the instruction
  // The program counter is incremented to point to the operand
  return pc++;
}
u16 CPU::ZeroPage() {
  // Zero page addressing
  // Returns the address from the zero page (0x0000 - 0x00FF).
  // The value of the next byte (operand) is the address in zero page memory
  return Read(pc++) & 0x00FF;
};
u16 CPU::Absolute() {
  // Absolute addressing
  // Constructs a 16 bit address from the next two bytes
  // The first byte (low byte) and the second byte (high byte) form the full
  // address.
  u16 lo = Read(pc++);
  u16 hi = Read(pc++);
  return (hi << 8) | lo;
}
u16 CPU::IndirectX() {
  // Indirect X addressing
  // Reads the next byte, which is a zero page address,
  // and adds the value of the x register to it.
  // The next two bytes form the 16 bit address.
  u8 zpAddress = Read(pc++);
  u8 offsetAddress = (zpAddress + x) & 0x00FF;
  u16 lo = Read(offsetAddress);
  u16 hi = Read((offsetAddress + 1) & 0x00FF);
  return (hi << 8) | lo;
}

// ----------------------------------------------------------------------------
// Opcodes --------------------------------------------------------------------
// ----------------------------------------------------------------------------
void CPU::BRK() { CPU::halt = true; }
void CPU::LDA(u16 (CPU::*addressingMode)()) {

  // Get the address
  u16 address = (this->*addressingMode)();

  // Read data into a
  a = Read(address);

  // Set status flags
  p &= ~(Zero | Negative); // Clear Zero and Negative flags
  if (a == 0) {
    p |= Zero; // Set Zero flag if a is zero
  }

  if (a & 0x80) {
    p |= Negative; // Set Negative flag if bit 7 is set
  }
}
void CPU::AND(u16 (CPU::*addressingMode)()) {
  // AND (bitwise AND with accumulator)
  u16 address = (this->*addressingMode)();
  a &= Read(address);

  // Clear Zero and Negative flags
  p &= ~(Zero | Negative);

  // Set Zero flag if a is zero
  if (a == 0) {
    p |= Zero;
  }

  // Set Negative flag if bit 7 is set
  if (a & 0x80) {
    p |= Negative;
  }
};

void CPU::STA(u16 (CPU::*addressingMode)()) {
  u16 address = (this->*addressingMode)();
  Write(address, a);
}

void CPU::Execute() {
  switch (Read(pc++)) {
  case 0x00:
    BRK();
    break;
  case 0x21: // AND (Indirect,X)
    AND(&CPU::IndirectX);
    break;
  case 0xA9: // LDA (Immediate)
    LDA(&CPU::Immediate);
    break;
  case 0x8D: // STA (Absolute)
    STA(&CPU::Absolute);
    break;
  default:
    std::cout << std::endl;
    std::cout << "EXECUTE WARNING, Invalid opcode: " << std::hex << std::setw(2)
              << std::setfill('0') << int(Read(pc - 1)) << std::endl;
    std::cout << std::endl;
  }
}
