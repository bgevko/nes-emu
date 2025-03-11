// 6502 Disassembler
#pragma once
#include <cstdint>
#include <string>
#include <vector>

using u8 = uint8_t;
using u16 = uint16_t;
using str = std::string;

using namespace std;

struct Instruction {
    str opcode;
    str name;
    str mode;
    str operand;
};

class Disassembler
{
  public:
    Disassembler() = default;

    str immPrefix = "#";
    str base16Prefix = "$";
    str base2Prefix = "%";
    // base 10: no prefix

    vector<Instruction> instructions;

    Instruction Parse( u8 opcode );
};
