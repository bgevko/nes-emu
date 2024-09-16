// cpu.cpp

#include "cpu.h"
#include <vector>

int main() {
  CPU cpu;

  // Test data
  std::vector<_8bit> data = {
      0xA9, 0x01, // LDA #$01 (Load the value 1 into the Accumulator)
      0x8D, 0x00, 0x02, // STA $0200 (Store the Accumulator at memory Address 0200)
      0x00 // BRK (Break)
  };

  cpu.LoadProgram(data, 0x0600);
  cpu.Reset();

  // Print the content of memory at the start of the program to verify loading
  cpu.PrintMemory(0x0600, 0x0600 + data.size() - 1);

  // Read the reset vector
  cpu.PrintMemory(0xFFFC, 0xFFFD);

  // Print the registers
  cpu.PrintRegisters();

  return 0;
}
