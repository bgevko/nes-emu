// cpu.cpp

#include "cpu.h"
#include <iomanip>
#include <iostream>

CPU::CPU() { Reset(); }

CPU::~CPU() {}

void CPU::LoadProgram(const std::vector<_8bit> &data, _16bit startAddress) {
  for (size_t i = 0; i < data.size(); i++) {
    if (Write(startAddress + i, data[i]) == -1) {
      std::cerr << "LoadProgram: Failed to write to address: " << std::hex
                << std::setw(4) << std::setfill('0') << startAddress + i
                << std::endl;
    }
  }

  // Set the reset vector to the start address
  int lo = Write(0xFFFC, startAddress & 0xFF);      // Low byte
  int hi = Write(0xFFFD, startAddress >> 8 & 0xFF); // High byte
  if (lo == -1 || hi == -1) {
    std::cerr << "LoadProgram: Failed to write to reset vector address: "
              << std::hex << std::setw(4) << std::setfill('0') << 0xFFFC
              << std::endl;
  }
}

void CPU::Reset() {
  A = 0x00;
  X = 0x00;
  Y = 0x00;
  Status = 0x00 | Unused;
  StackPointer = 0xFD;
  _8bit lo = Read(0xFFFC);
  _8bit hi = Read(0xFFFD);
  ProgramCounter = (_16bit(hi) << 8) | lo;
}

_8bit CPU::Read(_16bit address) {
  if (address >= 0x0000 && address <= 0xFFFF) {
    return RAM[address];
  }
  return -1;
}

int CPU::Write(_16bit address, _8bit data) {
  if (address >= 0x0000 && address <= 0xFFFF) {
    RAM[address] = data;
    return 0;
  }
  return -1;
}

void CPU::PrintMemory(_16bit start, _16bit end) {
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
  std::string status = "NV-BDIZC";
  std::string flags = "";
  for (int i = 7; i >= 0; i--) {
    flags += (Status & (1 << i)) ? "1" : "0";
  }
  return status + " " + flags;
}

void CPU::PrintRegisters() {
  std::cout << "A: " << std::hex << std::setw(2) << std::setfill('0') << int(A)
            << std::endl;
  std::cout << "X: " << std::hex << std::setw(2) << std::setfill('0') << int(X)
            << std::endl;
  std::cout << "Y: " << std::hex << std::setw(2) << std::setfill('0') << int(Y)
            << std::endl;
  std::cout << "Status: " << GetStatusString() << std::endl;

  std::cout << "Stack Pointer: " << std::hex << std::setw(2)
            << std::setfill('0') << int(StackPointer) << std::endl;
  std::cout << "Program Counter: " << std::hex << std::setw(4)
            << std::setfill('0') << int(ProgramCounter) << std::endl;
}

// Addressing Modes
_16bit CPU::IMM() { return ProgramCounter++; }
_16bit CPU::ZP0() { return Read(ProgramCounter++) & 0x00FF; };
_16bit CPU::ABS() {
  _16bit lo = Read(ProgramCounter++);
  _16bit hi = Read(ProgramCounter++);
  return (hi << 8) | lo;
}
// ----------------------------------------------------------------------------
// Opcodes --------------------------------------------------------------------
// ----------------------------------------------------------------------------
void CPU::BRK() { CPU::halt = true; }
void CPU::LDA(_16bit (CPU::*addressingMode)()) {

  // Get the address
  _16bit address = (this->*addressingMode)();

  // Read data into A
  A = Read(address);

  // Set flags
  Status &= ~(Zero | Negative); // Clear Zero and Negative flags
  if (A == 0) {
    Status |= Zero; // Set Zero flag if A is zero
  }

  if (A & 0x80) {
    Status |= Negative; // Set Negative flag if bit 7 is set
  }
}

void CPU::STA(_16bit (CPU::*addressingMode)()) {
  // Get the address
  _16bit address = (this->*addressingMode)();
  // Write A to memory
  Write(address, A);
}

void CPU::Execute() {
  switch (Read(ProgramCounter++)) {
  case 0x00:
    BRK();
    break;
  case 0xA9: // LDA (Immediate)
    return LDA(&CPU::IMM);
    break;
  case 0x8D: // STA (Absolute)
    return STA(&CPU::ABS);
    break;
  default:
    std::cout << "Invalid opcode:" << std::hex << std::setw(2)
              << std::setfill('0') << int(Read(ProgramCounter - 1))
              << std::endl;
  }
}
