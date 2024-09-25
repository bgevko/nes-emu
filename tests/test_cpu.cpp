#include "cpu.h"
#include "json.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>

bool VERBOSE = false;
using json = nlohmann::json;

// Helper function to load CPU state from JSON
void LoadStateFromJson(CPU &cpu, const json &j, const std::string &state) {
  cpu.pc = u16(j[state]["pc"]);
  cpu.a = j[state]["a"];
  cpu.x = j[state]["x"];
  cpu.y = j[state]["y"];
  cpu.s = j[state]["s"];
  cpu.p = j[state]["p"];
  for (const auto &ram_entry : j[state]["ram"]) {
    uint16_t address = ram_entry[0];
    uint8_t value = ram_entry[1];
    cpu.Write(address, value);
  }
}

// Helper function to print CPU state
void PrintCPUState(const CPU &cpu, const json &j, const std::string &state) {
  std::cout << "----------" << state << " State----------" << std::endl;
  std::cout << "pc: " << std::hex << std::setw(4) << std::setfill('0') << cpu.pc
            << std::endl;
  std::cout << "s: " << std::dec << int(cpu.s) << std::endl;
  std::cout << "a: " << int(cpu.a) << std::endl;
  std::cout << "x: " << int(cpu.x) << std::endl;
  std::cout << "y: " << int(cpu.y) << std::endl;
  std::cout << "p: " << int(cpu.p) << std::endl;
  std::cout << std::endl;
  std::cout << "RAM:" << std::endl;
  for (const auto &ram_entry : j[state]["ram"]) {
    uint16_t address = ram_entry[0];
    uint8_t value = cpu.Read(address);
    std::cout << std::hex << std::setw(4) << std::setfill('0') << address
              << ": " << std::hex << std::setw(2) << std::setfill('0')
              << int(value) << std::endl;
  }
  std::cout << "--------------------------------" << std::endl;
  std::cout << std::endl;
}

// Test case function for each test in the JSON array
void RunTestCase(const json &test_case) {
  // Initialize CPU
  CPU cpu;
  cpu.Reset();

  // Load initial state from JSON
  LoadStateFromJson(cpu, test_case, "initial");

  // Ensure loaded values match JSON values
  EXPECT_EQ(cpu.pc, u16(test_case["initial"]["pc"]));
  EXPECT_EQ(cpu.a, test_case["initial"]["a"]);
  EXPECT_EQ(cpu.x, test_case["initial"]["x"]);
  EXPECT_EQ(cpu.y, test_case["initial"]["y"]);
  EXPECT_EQ(cpu.s, test_case["initial"]["s"]);
  EXPECT_EQ(cpu.p, test_case["initial"]["p"]);
  for (const auto &ram_entry : test_case["initial"]["ram"]) {
    uint16_t address = ram_entry[0];
    uint8_t value = ram_entry[1];
    EXPECT_EQ(cpu.Read(address), value);
  }

  // Print initial state
  if (VERBOSE) {
    std::cout << "Running Test: " << test_case["name"] << std::endl;
    PrintCPUState(cpu, test_case, "initial");
  }

  // Execute instructions
  cpu.Execute();

  // Print final state
  if (VERBOSE) {
    PrintCPUState(cpu, test_case, "final");
  }

  // Ensure final values match JSON values
  EXPECT_EQ(cpu.pc, u16(test_case["final"]["pc"]))
      << "PC mismatch: Expected " << std::hex << std::setw(4)
      << std::setfill('0') << u16(test_case["final"]["pc"]) << ", but got "
      << cpu.pc;
  EXPECT_EQ(cpu.a, test_case["final"]["a"]);
  EXPECT_EQ(cpu.x, test_case["final"]["x"]);
  EXPECT_EQ(cpu.y, test_case["final"]["y"]);
  EXPECT_EQ(cpu.s, test_case["final"]["s"]);
  EXPECT_EQ(cpu.p, test_case["final"]["p"]);
  for (const auto &ram_entry : test_case["final"]["ram"]) {
    uint16_t address = ram_entry[0];
    uint8_t expected_value = ram_entry[1];
    uint8_t actual_value = cpu.Read(address);
    EXPECT_EQ(actual_value, expected_value)
        << "RAM mismatch at address " << std::hex << address << ": Expected "
        << int(expected_value) << ", but got " << int(actual_value);
  }

  std::cout << "Test " << test_case["name"] << " passed" << std::endl;
}

// GTest function to load and run all test cases from the JSON file
TEST(A9, LDAImmediate) {
  // Load the JSON array of test cases

  std::cout << "---------- A9 LDA Immediate Tests ---------" << std::endl;
  std::ifstream json_file("tests/HARTE/a9.json");
  if (!json_file.is_open()) {
    std::cerr << "Could not open test file: tests/small.json" << std::endl;
    FAIL();
  }

  json test_cases;
  json_file >> test_cases;

  // Ensure the file contains an array of test cases
  ASSERT_TRUE(test_cases.is_array())
      << "Expected an array of test cases in JSON file";

  // Loop through each test case in the JSON array and run the test
  for (const auto &test_case : test_cases) {
    RunTestCase(test_case);
  }

  std::cout << "---------- A9 LDA Immediate Tests Complete ---------"
            << std::endl;
  std::cout << std::endl;
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
