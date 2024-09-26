// cpu.cpp

#include "cpu.h"
#include <vector>

int main() {
  CPU cpu;

  // LDA (Immediate)
  std::vector<u8> data = {
      0xA9, 0xFF, // LDA #$FF (Load value into the Accumulator)
      0x8D, 0x00,
      0x02, // STA $0200 (Store the Accumulator at memory Address 0200)
      0x00  // BRK (Break)
  };

  cpu.Reset();
  cpu.LoadProgram(data, 0x0600);

  // For now, run until the BRK instruction is encountered
  while (!cpu.halt) {
    cpu.FetchDecodeExecute();
  }

  // print the affected memory locations
  cpu.PrintMemory(0x0200);
  cpu.PrintMemory(0x0600, 0x0600 + data.size() - 1);

  // Print the registers
  cpu.PrintRegisters();

  return 0;
}
