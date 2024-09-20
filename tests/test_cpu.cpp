#include "cpu.h"
#include <gtest/gtest.h>

// LDA Immediate
TEST(CPUInstructions, LDAImmediate) {
  CPU cpu;
  cpu.LoadProgram({0xA9, 0x01}); // LDA #$01
  cpu.Reset();
  cpu.Execute();

  EXPECT_EQ(cpu.A, 0x01);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
