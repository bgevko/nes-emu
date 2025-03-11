#pragma once

#include <array>
#include <deque>
#include <cstdint>
#include <string>

// Aliases for integer types
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8 = int8_t;

// Static arrays for opcode information
// clang-format off
const std::array<std::string, 256> gInstructionNames = {
    "BRK",   "ORA",   "*JAM",  "*SLO",  "*NOP",  "ORA",   "*ASL",  "*SLO",  "PHP",  "ORA",   "ASL",   "*ANC",  "*NOP",  "ORA",   "ASL",   "*SLO",
    "BPL",   "ORA",   "*JAM",  "*SLO",  "*NOP",  "ORA",   "*ASL",  "*SLO",  "CLC",  "ORA",   "*NOP",  "*SLO",  "*NOP",  "ORA",   "ASL",   "*SLO",
    "JSR",   "AND",   "*JAM",  "*RLA",  "BIT",   "AND",   "ROL",   "*RLA",  "PLP",  "AND",   "ROL",   "*ANC",  "*NOP",  "AND",   "ROL",   "*RLA",
    "BMI",   "AND",   "*JAM",  "*RLA",  "*NOP",  "AND",   "*ROL",  "*RLA",  "SEC",  "AND",   "*NOP",  "*RLA",  "*NOP",  "AND",   "ROL",   "*RLA",
    "RTI",   "EOR",   "*JAM",  "*SRE",  "*NOP",  "EOR",   "*LSR",  "*SRE",  "PHA",  "EOR",   "LSR",   "*ALR",  "*JMP",  "EOR",   "LSR",   "*SRE",
    "BVC",   "EOR",   "*JAM",  "*SRE",  "*NOP",  "EOR",   "*LSR",  "*SRE",  "CLI",  "EOR",   "*NOP",  "*SRE",  "*NOP",  "EOR",   "LSR",   "*SRE",
    "RTS",   "ADC",   "*JAM",  "*RRA",  "*NOP",  "ADC",   "*ROR",  "*RRA",  "PLA",  "ADC",   "ROR",   "*ARR",  "JMP",   "ADC",   "ROR",   "*RRA",
    "BVS",   "ADC",   "*JAM",  "*RRA",  "*NOP",  "ADC",   "*ROR",  "*RRA",  "SEI",  "ADC",   "*NOP",  "*RRA",  "*NOP",  "ADC",   "ROR",   "*RRA",
    "*NOP",  "STA",   "*NOP",  "*SAX",  "STY",   "STA",   "STX",   "*SAX",  "DEY",  "*NOP",  "TXA",   "*ANE",  "STY",   "STA",   "STX",   "*SAX",
    "BCC",   "STA",   "*JAM",  "*SHA",  "STY",   "STA",   "STX",   "*STX",  "TYA",  "STA",   "TXS",   "*TAS",  "*SHY",  "STA",   "*SHX",  "*SHA",
    "LDY",   "LDA",   "LDX",   "*LAX",  "LDY",   "LDA",   "LDX",   "*LAX",  "TAY",  "LDA",   "TAX",   "*LXA",  "LDY",   "LDA",   "LDX",   "*LAX",
    "BCS",   "LDA",   "*JAM",  "*LAX",  "LDY",   "LDA",   "LDX",   "*LAX",  "CLV",  "LDA",   "TSX",   "*LAS",  "LDY",   "LDA",   "LDX",   "*LAX",
    "CPY",   "CMP",   "*NOP",  "*DCP",  "CPY",   "CMP",   "DEC",   "*DCP",  "INY",  "CMP",   "DEX",   "*SBX",  "CPY",   "CMP",   "DEC",   "*DCP",
    "BNE",   "CMP",   "*JAM",  "*DCP",  "*NOP",  "CMP",   "*DEC",  "*DCP",  "CLD",  "CMP",   "*NOP",  "*DCP",  "*NOP",  "CMP",   "DEC",   "*DCP",
    "CPX",   "SBC",   "*NOP",  "*ISC",  "CPX",   "SBC",   "INC",   "*ISC",  "INX",  "SBC",   "NOP",   "*SBC",  "CPX",   "SBC",   "INC",   "*ISC",
    "BEQ",   "SBC",   "*JAM",  "*ISC",  "*NOP",  "SBC",   "*INC",  "*ISC",  "SED",  "SBC",   "*NOP",  "*ISC",  "*NOP",  "SBC",   "INC",   "*ISC"
};

const std::array<std::string, 256> gAddressingModes = {
    "IMP",   "INDX",  "IMP",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
    "REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX",
    "ABS",   "INDX",  "IMP",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
    "REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX",
    "IMP",   "INDX",  "IMP",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
    "REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX",
    "IMP",   "INDX",  "IMP",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "IND",   "ABS",   "ABS",   "ABS",
    "REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX",
    "IMM",   "INDX",  "IMM",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
    "REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGY",  "ZPGY",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSY",  "ABSY",
    "IMM",   "INDX",  "IMM",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
    "REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGY",  "ZPGY",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSY",  "ABSY",
    "IMM",   "INDX",  "IMM",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
    "REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX",
    "IMM",   "INDX",  "IMM",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
    "REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX"
};

const std::array<u8, 256> gInstructionCycles = {
    7,      6,      2,      8,      3,      3,      5,      5,      3,      2,      2,      2,      4,      4,      6,      6,
    2,      5,      2,      8,      4,      4,      6,      6,      2,      4,      2,      7,      4,      4,      7,      7,
    6,      6,      2,      8,      3,      3,      5,      5,      4,      2,      2,      2,      4,      4,      6,      6,
    2,      5,      2,      8,      4,      4,      6,      6,      2,      4,      2,      7,      4,      4,      7,      7,
    6,      6,      2,      8,      3,      3,      5,      5,      3,      2,      2,      2,      3,      4,      6,      6,
    2,      5,      2,      8,      4,      4,      6,      6,      2,      4,      2,      7,      4,      4,      7,      7,
    6,      6,      2,      8,      3,      3,      5,      5,      4,      2,      2,      2,      5,      4,      6,      6,
    2,      5,      2,      8,      4,      4,      6,      6,      2,      4,      2,      7,      4,      4,      7,      7,
    2,      6,      2,      6,      3,      3,      3,      3,      2,      2,      2,      2,      4,      4,      4,      4,
    2,      6,      2,      6,      4,      4,      4,      4,      2,      5,      2,      5,      5,      5,      5,      5,
    2,      6,      2,      6,      3,      3,      3,      3,      2,      2,      2,      2,      4,      4,      4,      4,
    2,      5,      2,      5,      4,      4,      4,      4,      2,      4,      2,      4,      4,      4,      4,      4,
    2,      6,      2,      8,      3,      3,      5,      5,      2,      2,      2,      2,      4,      4,      6,      6,
    2,      5,      2,      8,      4,      4,      6,      6,      2,      4,      2,      7,      4,      4,      7,      7,
    2,      6,      2,      8,      3,      3,      5,      5,      2,      2,      2,      2,      4,      4,      6,      6,
    2,      5,      2,      8,      4,      4,      6,      6,      2,      4,      2,      7,      4,      4,      7,      7
};

const std::array<u8, 256> gInstructionBytes = {
    1,      2,      1,      2,      2,      2,      2,      2,      1,      2,      1,      2,      3,      3,      3,      3,
    2,      2,      1,      2,      2,      2,      2,      2,      1,      3,      1,      3,      3,      3,      3,      3,
    3,      2,      1,      2,      2,      2,      2,      2,      1,      2,      1,      2,      3,      3,      3,      3,
    2,      2,      1,      2,      2,      2,      2,      2,      1,      3,      1,      3,      3,      3,      3,      3,
    1,      2,      1,      2,      2,      2,      2,      2,      1,      2,      1,      2,      3,      3,      3,      3,
    2,      2,      1,      2,      2,      2,      2,      2,      1,      3,      1,      3,      3,      3,      3,      3,
    1,      2,      1,      2,      2,      2,      2,      2,      1,      2,      1,      2,      3,      3,      3,      3,
    2,      2,      1,      2,      2,      2,      2,      2,      1,      3,      1,      3,      3,      3,      3,      3,
    2,      2,      2,      2,      2,      2,      2,      2,      1,      2,      1,      2,      3,      3,      3,      3,
    2,      2,      1,      2,      2,      2,      2,      2,      1,      3,      1,      3,      3,      3,      3,      3,
    2,      2,      2,      2,      2,      2,      2,      2,      1,      2,      1,      2,      3,      3,      3,      3,
    2,      2,      1,      2,      2,      2,      2,      2,      1,      3,      1,      3,      3,      3,      3,      3,
    2,      2,      2,      2,      2,      2,      2,      2,      1,      2,      1,      2,      3,      3,      3,      3,
    2,      2,      1,      2,      2,      2,      2,      2,      1,      3,      1,      3,      3,      3,      3,      3,
    2,      2,      2,      2,      2,      2,      2,      2,      1,      2,      1,      2,      3,      3,      3,      3,
    2,      2,      1,      2,      2,      2,      2,      2,      1,      3,      1,      3,      3,      3,      3,      3
};
// clang-format on

// Forward declaration for reads and writes
class Bus;

class CPU
{
  public:
    explicit CPU( Bus *bus ) : _bus( bus )
    {

        /*
        ################################################################
        ||                                                            ||
        ||                           Opcodes                          ||
        ||                                                            ||
        ################################################################
        */
        // NOP
        _opcodeTable[0xEA] = InstructionData{ &CPU::NOP, &CPU::IMP };

        // LDA
        _opcodeTable[0xA9] = InstructionData{ &CPU::LDA, &CPU::IMM };
        _opcodeTable[0xA5] = InstructionData{ &CPU::LDA, &CPU::ZPG };
        _opcodeTable[0xB5] = InstructionData{ &CPU::LDA, &CPU::ZPGX };
        _opcodeTable[0xAD] = InstructionData{ &CPU::LDA, &CPU::ABS };
        _opcodeTable[0xBD] = InstructionData{ &CPU::LDA, &CPU::ABSX };
        _opcodeTable[0xB9] = InstructionData{ &CPU::LDA, &CPU::ABSY };
        _opcodeTable[0xA1] = InstructionData{ &CPU::LDA, &CPU::INDX };
        _opcodeTable[0xB1] = InstructionData{ &CPU::LDA, &CPU::INDY };

        // LDX
        _opcodeTable[0xA2] = InstructionData{ &CPU::LDX, &CPU::IMM };
        _opcodeTable[0xA6] = InstructionData{ &CPU::LDX, &CPU::ZPG };
        _opcodeTable[0xB6] = InstructionData{ &CPU::LDX, &CPU::ZPGY, true, true };
        _opcodeTable[0xAE] = InstructionData{ &CPU::LDX, &CPU::ABS };
        _opcodeTable[0xBE] = InstructionData{ &CPU::LDX, &CPU::ABSY };

        // LDY
        _opcodeTable[0xA0] = InstructionData{ &CPU::LDY, &CPU::IMM };
        _opcodeTable[0xA4] = InstructionData{ &CPU::LDY, &CPU::ZPG };
        _opcodeTable[0xB4] = InstructionData{ &CPU::LDY, &CPU::ZPGX };
        _opcodeTable[0xAC] = InstructionData{ &CPU::LDY, &CPU::ABS };
        _opcodeTable[0xBC] = InstructionData{ &CPU::LDY, &CPU::ABSX };

        // STA
        _opcodeTable[0x85] = InstructionData{ &CPU::STA, &CPU::ZPG };
        _opcodeTable[0x95] = InstructionData{ &CPU::STA, &CPU::ZPGX };
        _opcodeTable[0x8D] = InstructionData{ &CPU::STA, &CPU::ABS };
        _opcodeTable[0x9D] = InstructionData{ &CPU::STA, &CPU::ABSX, false, true };
        _opcodeTable[0x99] = InstructionData{ &CPU::STA, &CPU::ABSY, false, true };
        _opcodeTable[0x81] = InstructionData{ &CPU::STA, &CPU::INDX, false };
        _opcodeTable[0x91] = InstructionData{ &CPU::STA, &CPU::INDY, false, true };

        // STX
        _opcodeTable[0x86] = InstructionData{ &CPU::STX, &CPU::ZPG };
        _opcodeTable[0x96] = InstructionData{ &CPU::STX, &CPU::ZPGY, true, true };
        _opcodeTable[0x8E] = InstructionData{ &CPU::STX, &CPU::ABS };

        // STY
        _opcodeTable[0x84] = InstructionData{ &CPU::STY, &CPU::ZPG };
        _opcodeTable[0x94] = InstructionData{ &CPU::STY, &CPU::ZPGX };
        _opcodeTable[0x8C] = InstructionData{ &CPU::STY, &CPU::ABS };

        // ADC
        _opcodeTable[0x69] = InstructionData{ &CPU::ADC, &CPU::IMM };
        _opcodeTable[0x65] = InstructionData{ &CPU::ADC, &CPU::ZPG };
        _opcodeTable[0x75] = InstructionData{ &CPU::ADC, &CPU::ZPGX };
        _opcodeTable[0x6D] = InstructionData{ &CPU::ADC, &CPU::ABS };
        _opcodeTable[0x7D] = InstructionData{ &CPU::ADC, &CPU::ABSX };
        _opcodeTable[0x79] = InstructionData{ &CPU::ADC, &CPU::ABSY };
        _opcodeTable[0x61] = InstructionData{ &CPU::ADC, &CPU::INDX };
        _opcodeTable[0x71] = InstructionData{ &CPU::ADC, &CPU::INDY };

        // SBC
        _opcodeTable[0xE9] = InstructionData{ &CPU::SBC, &CPU::IMM };
        _opcodeTable[0xE5] = InstructionData{ &CPU::SBC, &CPU::ZPG };
        _opcodeTable[0xF5] = InstructionData{ &CPU::SBC, &CPU::ZPGX };
        _opcodeTable[0xED] = InstructionData{ &CPU::SBC, &CPU::ABS };
        _opcodeTable[0xFD] = InstructionData{ &CPU::SBC, &CPU::ABSX };
        _opcodeTable[0xF9] = InstructionData{ &CPU::SBC, &CPU::ABSY };
        _opcodeTable[0xE1] = InstructionData{ &CPU::SBC, &CPU::INDX };
        _opcodeTable[0xF1] = InstructionData{ &CPU::SBC, &CPU::INDY };

        // INC
        _opcodeTable[0xE6] = InstructionData{ &CPU::INC, &CPU::ZPG };
        _opcodeTable[0xF6] = InstructionData{ &CPU::INC, &CPU::ZPGX };
        _opcodeTable[0xEE] = InstructionData{ &CPU::INC, &CPU::ABS };
        _opcodeTable[0xFE] = InstructionData{ &CPU::INC, &CPU::ABSX, false, true };

        // DEC
        _opcodeTable[0xC6] = InstructionData{ &CPU::DEC, &CPU::ZPG };
        _opcodeTable[0xD6] = InstructionData{ &CPU::DEC, &CPU::ZPGX };
        _opcodeTable[0xCE] = InstructionData{ &CPU::DEC, &CPU::ABS };
        _opcodeTable[0xDE] = InstructionData{ &CPU::DEC, &CPU::ABSX, false, true };

        // INX, INY, DEX, DEY
        _opcodeTable[0xE8] = InstructionData{ &CPU::INX, &CPU::IMP };
        _opcodeTable[0xC8] = InstructionData{ &CPU::INY, &CPU::IMP };
        _opcodeTable[0xCA] = InstructionData{ &CPU::DEX, &CPU::IMP };
        _opcodeTable[0x88] = InstructionData{ &CPU::DEY, &CPU::IMP };

        // CLC
        _opcodeTable[0x18] = InstructionData{ &CPU::CLC, &CPU::IMP };
        _opcodeTable[0x58] = InstructionData{ &CPU::CLI, &CPU::IMP };
        _opcodeTable[0xD8] = InstructionData{ &CPU::CLD, &CPU::IMP };
        _opcodeTable[0xB8] = InstructionData{ &CPU::CLV, &CPU::IMP };

        _opcodeTable[0x38] = InstructionData{ &CPU::SEC, &CPU::IMP };
        _opcodeTable[0x78] = InstructionData{ &CPU::SEI, &CPU::IMP };
        _opcodeTable[0xF8] = InstructionData{ &CPU::SED, &CPU::IMP };

        // Branch
        _opcodeTable[0x10] = InstructionData{ &CPU::BPL, &CPU::REL };
        _opcodeTable[0x30] = InstructionData{ &CPU::BMI, &CPU::REL };
        _opcodeTable[0x50] = InstructionData{ &CPU::BVC, &CPU::REL };
        _opcodeTable[0x70] = InstructionData{ &CPU::BVS, &CPU::REL };
        _opcodeTable[0x90] = InstructionData{ &CPU::BCC, &CPU::REL };
        _opcodeTable[0xB0] = InstructionData{ &CPU::BCS, &CPU::REL };
        _opcodeTable[0xD0] = InstructionData{ &CPU::BNE, &CPU::REL };
        _opcodeTable[0xF0] = InstructionData{ &CPU::BEQ, &CPU::REL };

        // CMP, CPX, CPY
        _opcodeTable[0xC9] = InstructionData{ &CPU::CMP, &CPU::IMM };
        _opcodeTable[0xC5] = InstructionData{ &CPU::CMP, &CPU::ZPG };
        _opcodeTable[0xD5] = InstructionData{ &CPU::CMP, &CPU::ZPGX };
        _opcodeTable[0xCD] = InstructionData{ &CPU::CMP, &CPU::ABS };
        _opcodeTable[0xDD] = InstructionData{ &CPU::CMP, &CPU::ABSX };
        _opcodeTable[0xD9] = InstructionData{ &CPU::CMP, &CPU::ABSY };
        _opcodeTable[0xC1] = InstructionData{ &CPU::CMP, &CPU::INDX };
        _opcodeTable[0xD1] = InstructionData{ &CPU::CMP, &CPU::INDY };
        _opcodeTable[0xE0] = InstructionData{ &CPU::CPX, &CPU::IMM };
        _opcodeTable[0xE4] = InstructionData{ &CPU::CPX, &CPU::ZPG };
        _opcodeTable[0xEC] = InstructionData{ &CPU::CPX, &CPU::ABS };
        _opcodeTable[0xC0] = InstructionData{ &CPU::CPY, &CPU::IMM };
        _opcodeTable[0xC4] = InstructionData{ &CPU::CPY, &CPU::ZPG };
        _opcodeTable[0xCC] = InstructionData{ &CPU::CPY, &CPU::ABS };

        // PHA, PHP, PLA, PLP, TSX, TXS
        _opcodeTable[0x48] = InstructionData{ &CPU::PHA, &CPU::IMP };
        _opcodeTable[0x08] = InstructionData{ &CPU::PHP, &CPU::IMP };
        _opcodeTable[0x68] = InstructionData{ &CPU::PLA, &CPU::IMP };
        _opcodeTable[0x28] = InstructionData{ &CPU::PLP, &CPU::IMP };
        _opcodeTable[0xBA] = InstructionData{ &CPU::TSX, &CPU::IMP };
        _opcodeTable[0x9A] = InstructionData{ &CPU::TXS, &CPU::IMP };

        // ASL, LSR
        _opcodeTable[0x0A] = InstructionData{ &CPU::ASL, &CPU::IMP };
        _opcodeTable[0x06] = InstructionData{ &CPU::ASL, &CPU::ZPG };
        _opcodeTable[0xA9] = InstructionData{ &CPU::LDA, &CPU::IMM };
        _opcodeTable[0xA5] = InstructionData{ &CPU::LDA, &CPU::ZPG };
        _opcodeTable[0xB5] = InstructionData{ &CPU::LDA, &CPU::ZPGX };
        _opcodeTable[0xAD] = InstructionData{ &CPU::LDA, &CPU::ABS };
        _opcodeTable[0xBD] = InstructionData{ &CPU::LDA, &CPU::ABSX };
        _opcodeTable[0xB9] = InstructionData{ &CPU::LDA, &CPU::ABSY };
        _opcodeTable[0xA1] = InstructionData{ &CPU::LDA, &CPU::INDX };
        _opcodeTable[0xB1] = InstructionData{ &CPU::LDA, &CPU::INDY };

        // LDX
        _opcodeTable[0xA2] = InstructionData{ &CPU::LDX, &CPU::IMM };
        _opcodeTable[0xA6] = InstructionData{ &CPU::LDX, &CPU::ZPG };
        _opcodeTable[0xB6] = InstructionData{ &CPU::LDX, &CPU::ZPGY, true, true };
        _opcodeTable[0xAE] = InstructionData{ &CPU::LDX, &CPU::ABS };
        _opcodeTable[0xBE] = InstructionData{ &CPU::LDX, &CPU::ABSY };

        // LDY
        _opcodeTable[0xA0] = InstructionData{ &CPU::LDY, &CPU::IMM };
        _opcodeTable[0xA4] = InstructionData{ &CPU::LDY, &CPU::ZPG };
        _opcodeTable[0xB4] = InstructionData{ &CPU::LDY, &CPU::ZPGX };
        _opcodeTable[0xAC] = InstructionData{ &CPU::LDY, &CPU::ABS };
        _opcodeTable[0xBC] = InstructionData{ &CPU::LDY, &CPU::ABSX };

        // STA
        _opcodeTable[0x85] = InstructionData{ &CPU::STA, &CPU::ZPG };
        _opcodeTable[0x95] = InstructionData{ &CPU::STA, &CPU::ZPGX };
        _opcodeTable[0x8D] = InstructionData{ &CPU::STA, &CPU::ABS };
        _opcodeTable[0x9D] = InstructionData{ &CPU::STA, &CPU::ABSX, false, true };
        _opcodeTable[0x99] = InstructionData{ &CPU::STA, &CPU::ABSY, false, true };
        _opcodeTable[0x81] = InstructionData{ &CPU::STA, &CPU::INDX, false };
        _opcodeTable[0x91] = InstructionData{ &CPU::STA, &CPU::INDY, false, true };

        // STX
        _opcodeTable[0x86] = InstructionData{ &CPU::STX, &CPU::ZPG };
        _opcodeTable[0x96] = InstructionData{ &CPU::STX, &CPU::ZPGY, true, true };
        _opcodeTable[0x8E] = InstructionData{ &CPU::STX, &CPU::ABS };

        // STY
        _opcodeTable[0x84] = InstructionData{ &CPU::STY, &CPU::ZPG };
        _opcodeTable[0x94] = InstructionData{ &CPU::STY, &CPU::ZPGX };
        _opcodeTable[0x8C] = InstructionData{ &CPU::STY, &CPU::ABS };

        // ADC
        _opcodeTable[0x69] = InstructionData{ &CPU::ADC, &CPU::IMM };
        _opcodeTable[0x65] = InstructionData{ &CPU::ADC, &CPU::ZPG };
        _opcodeTable[0x75] = InstructionData{ &CPU::ADC, &CPU::ZPGX };
        _opcodeTable[0x6D] = InstructionData{ &CPU::ADC, &CPU::ABS };
        _opcodeTable[0x7D] = InstructionData{ &CPU::ADC, &CPU::ABSX };
        _opcodeTable[0x79] = InstructionData{ &CPU::ADC, &CPU::ABSY };
        _opcodeTable[0x61] = InstructionData{ &CPU::ADC, &CPU::INDX };
        _opcodeTable[0x71] = InstructionData{ &CPU::ADC, &CPU::INDY };

        // SBC
        _opcodeTable[0xE9] = InstructionData{ &CPU::SBC, &CPU::IMM };
        _opcodeTable[0xE5] = InstructionData{ &CPU::SBC, &CPU::ZPG };
        _opcodeTable[0xF5] = InstructionData{ &CPU::SBC, &CPU::ZPGX };
        _opcodeTable[0xED] = InstructionData{ &CPU::SBC, &CPU::ABS };
        _opcodeTable[0xFD] = InstructionData{ &CPU::SBC, &CPU::ABSX };
        _opcodeTable[0xF9] = InstructionData{ &CPU::SBC, &CPU::ABSY };
        _opcodeTable[0xE1] = InstructionData{ &CPU::SBC, &CPU::INDX };
        _opcodeTable[0xF1] = InstructionData{ &CPU::SBC, &CPU::INDY };

        // INC
        _opcodeTable[0xE6] = InstructionData{ &CPU::INC, &CPU::ZPG };
        _opcodeTable[0xF6] = InstructionData{ &CPU::INC, &CPU::ZPGX };
        _opcodeTable[0xEE] = InstructionData{ &CPU::INC, &CPU::ABS };
        _opcodeTable[0xFE] = InstructionData{ &CPU::INC, &CPU::ABSX, false, true };

        // DEC
        _opcodeTable[0xC6] = InstructionData{ &CPU::DEC, &CPU::ZPG };
        _opcodeTable[0xD6] = InstructionData{ &CPU::DEC, &CPU::ZPGX };
        _opcodeTable[0xCE] = InstructionData{ &CPU::DEC, &CPU::ABS };
        _opcodeTable[0xDE] = InstructionData{ &CPU::DEC, &CPU::ABSX, false, true };

        // INX, INY, DEX, DEY
        _opcodeTable[0xE8] = InstructionData{ &CPU::INX, &CPU::IMP };
        _opcodeTable[0xC8] = InstructionData{ &CPU::INY, &CPU::IMP };
        _opcodeTable[0xCA] = InstructionData{ &CPU::DEX, &CPU::IMP };
        _opcodeTable[0x88] = InstructionData{ &CPU::DEY, &CPU::IMP };

        // CLC
        _opcodeTable[0x18] = InstructionData{ &CPU::CLC, &CPU::IMP };
        _opcodeTable[0x58] = InstructionData{ &CPU::CLI, &CPU::IMP };
        _opcodeTable[0xD8] = InstructionData{ &CPU::CLD, &CPU::IMP };
        _opcodeTable[0xB8] = InstructionData{ &CPU::CLV, &CPU::IMP };

        _opcodeTable[0x38] = InstructionData{ &CPU::SEC, &CPU::IMP };
        _opcodeTable[0x78] = InstructionData{ &CPU::SEI, &CPU::IMP };
        _opcodeTable[0xF8] = InstructionData{ &CPU::SED, &CPU::IMP };

        // Branch
        _opcodeTable[0x10] = InstructionData{ &CPU::BPL, &CPU::REL };
        _opcodeTable[0x30] = InstructionData{ &CPU::BMI, &CPU::REL };
        _opcodeTable[0x50] = InstructionData{ &CPU::BVC, &CPU::REL };
        _opcodeTable[0x70] = InstructionData{ &CPU::BVS, &CPU::REL };
        _opcodeTable[0x90] = InstructionData{ &CPU::BCC, &CPU::REL };
        _opcodeTable[0xB0] = InstructionData{ &CPU::BCS, &CPU::REL };
        _opcodeTable[0xD0] = InstructionData{ &CPU::BNE, &CPU::REL };
        _opcodeTable[0xF0] = InstructionData{ &CPU::BEQ, &CPU::REL };

        // CMP, CPX, CPY
        _opcodeTable[0xC9] = InstructionData{ &CPU::CMP, &CPU::IMM };
        _opcodeTable[0xC5] = InstructionData{ &CPU::CMP, &CPU::ZPG };
        _opcodeTable[0xD5] = InstructionData{ &CPU::CMP, &CPU::ZPGX };
        _opcodeTable[0xCD] = InstructionData{ &CPU::CMP, &CPU::ABS };
        _opcodeTable[0xDD] = InstructionData{ &CPU::CMP, &CPU::ABSX };
        _opcodeTable[0xD9] = InstructionData{ &CPU::CMP, &CPU::ABSY };
        _opcodeTable[0xC1] = InstructionData{ &CPU::CMP, &CPU::INDX };
        _opcodeTable[0xD1] = InstructionData{ &CPU::CMP, &CPU::INDY };
        _opcodeTable[0xE0] = InstructionData{ &CPU::CPX, &CPU::IMM };
        _opcodeTable[0xE4] = InstructionData{ &CPU::CPX, &CPU::ZPG };
        _opcodeTable[0xEC] = InstructionData{ &CPU::CPX, &CPU::ABS };
        _opcodeTable[0xC0] = InstructionData{ &CPU::CPY, &CPU::IMM };
        _opcodeTable[0xC4] = InstructionData{ &CPU::CPY, &CPU::ZPG };
        _opcodeTable[0xCC] = InstructionData{ &CPU::CPY, &CPU::ABS };

        // PHA, PHP, PLA, PLP, TSX, TXS
        _opcodeTable[0x48] = InstructionData{ &CPU::PHA, &CPU::IMP };
        _opcodeTable[0x08] = InstructionData{ &CPU::PHP, &CPU::IMP };
        _opcodeTable[0x68] = InstructionData{ &CPU::PLA, &CPU::IMP };
        _opcodeTable[0x28] = InstructionData{ &CPU::PLP, &CPU::IMP };
        _opcodeTable[0xBA] = InstructionData{ &CPU::TSX, &CPU::IMP };
        _opcodeTable[0x9A] = InstructionData{ &CPU::TXS, &CPU::IMP };

        // ASL, LSR
        _opcodeTable[0x0A] = InstructionData{ &CPU::ASL, &CPU::IMP };
        _opcodeTable[0x06] = InstructionData{ &CPU::ASL, &CPU::ZPG };
        _opcodeTable[0x16] = InstructionData{ &CPU::ASL, &CPU::ZPGX };
        _opcodeTable[0x0E] = InstructionData{ &CPU::ASL, &CPU::ABS };
        _opcodeTable[0x1E] = InstructionData{ &CPU::ASL, &CPU::ABSX, false, true };
        _opcodeTable[0x4A] = InstructionData{ &CPU::LSR, &CPU::IMP };
        _opcodeTable[0x46] = InstructionData{ &CPU::LSR, &CPU::ZPG };
        _opcodeTable[0x56] = InstructionData{ &CPU::LSR, &CPU::ZPGX };
        _opcodeTable[0x4E] = InstructionData{ &CPU::LSR, &CPU::ABS };
        _opcodeTable[0x5E] = InstructionData{ &CPU::LSR, &CPU::ABSX, false, true };

        // ROL, ROR
        _opcodeTable[0x2A] = InstructionData{ &CPU::ROL, &CPU::IMP };
        _opcodeTable[0x26] = InstructionData{ &CPU::ROL, &CPU::ZPG };
        _opcodeTable[0x36] = InstructionData{ &CPU::ROL, &CPU::ZPGX };
        _opcodeTable[0x2E] = InstructionData{ &CPU::ROL, &CPU::ABS };
        _opcodeTable[0x3E] = InstructionData{ &CPU::ROL, &CPU::ABSX, false, true };
        _opcodeTable[0x6A] = InstructionData{ &CPU::ROR, &CPU::IMP };
        _opcodeTable[0x66] = InstructionData{ &CPU::ROR, &CPU::ZPG };
        _opcodeTable[0x76] = InstructionData{ &CPU::ROR, &CPU::ZPGX };
        _opcodeTable[0x6E] = InstructionData{ &CPU::ROR, &CPU::ABS };
        _opcodeTable[0x7E] = InstructionData{ &CPU::ROR, &CPU::ABSX, false, true };

        // JMP JSR, RTS, RTI, BRK
        _opcodeTable[0x4C] = InstructionData{ &CPU::JMP, &CPU::ABS };
        _opcodeTable[0x6C] = InstructionData{ &CPU::JMP, &CPU::IND };
        _opcodeTable[0x20] = InstructionData{ &CPU::JSR, &CPU::ABS };
        _opcodeTable[0x60] = InstructionData{ &CPU::RTS, &CPU::IMP };
        _opcodeTable[0x00] = InstructionData{ &CPU::BRK, &CPU::IMP };
        _opcodeTable[0x40] = InstructionData{ &CPU::RTI, &CPU::IMP };

        // AND
        _opcodeTable[0x29] = InstructionData{ &CPU::AND, &CPU::IMM };
        _opcodeTable[0x25] = InstructionData{ &CPU::AND, &CPU::ZPG };
        _opcodeTable[0x35] = InstructionData{ &CPU::AND, &CPU::ZPGX };
        _opcodeTable[0x2D] = InstructionData{ &CPU::AND, &CPU::ABS };
        _opcodeTable[0x3D] = InstructionData{ &CPU::AND, &CPU::ABSX };
        _opcodeTable[0x39] = InstructionData{ &CPU::AND, &CPU::ABSY };
        _opcodeTable[0x21] = InstructionData{ &CPU::AND, &CPU::INDX };
        _opcodeTable[0x31] = InstructionData{ &CPU::AND, &CPU::INDY };

        // ORA
        _opcodeTable[0x09] = InstructionData{ &CPU::ORA, &CPU::IMM };
        _opcodeTable[0x05] = InstructionData{ &CPU::ORA, &CPU::ZPG };
        _opcodeTable[0x15] = InstructionData{ &CPU::ORA, &CPU::ZPGX };
        _opcodeTable[0x0D] = InstructionData{ &CPU::ORA, &CPU::ABS };
        _opcodeTable[0x1D] = InstructionData{ &CPU::ORA, &CPU::ABSX };
        _opcodeTable[0x19] = InstructionData{ &CPU::ORA, &CPU::ABSY };
        _opcodeTable[0x01] = InstructionData{ &CPU::ORA, &CPU::INDX };
        _opcodeTable[0x11] = InstructionData{ &CPU::ORA, &CPU::INDY };

        // EOR
        _opcodeTable[0x49] = InstructionData{ &CPU::EOR, &CPU::IMM };
        _opcodeTable[0x45] = InstructionData{ &CPU::EOR, &CPU::ZPG };
        _opcodeTable[0x55] = InstructionData{ &CPU::EOR, &CPU::ZPGX };
        _opcodeTable[0x4D] = InstructionData{ &CPU::EOR, &CPU::ABS };
        _opcodeTable[0x5D] = InstructionData{ &CPU::EOR, &CPU::ABSX };
        _opcodeTable[0x59] = InstructionData{ &CPU::EOR, &CPU::ABSY };
        _opcodeTable[0x41] = InstructionData{ &CPU::EOR, &CPU::INDX };
        _opcodeTable[0x51] = InstructionData{ &CPU::EOR, &CPU::INDY };

        // BIT
        _opcodeTable[0x24] = InstructionData{ &CPU::BIT, &CPU::ZPG };
        _opcodeTable[0x2C] = InstructionData{ &CPU::BIT, &CPU::ABS };

        // Transfer
        _opcodeTable[0xAA] = InstructionData{ &CPU::TAX, &CPU::IMP };
        _opcodeTable[0x8A] = InstructionData{ &CPU::TXA, &CPU::IMP };
        _opcodeTable[0xA8] = InstructionData{ &CPU::TAY, &CPU::IMP };
        _opcodeTable[0x98] = InstructionData{ &CPU::TYA, &CPU::IMP };

        /*
        ################################
        ||       Illegal Opcodes      ||
        ################################
        */
        // Jams (does nothing)
        _opcodeTable[0x02] = InstructionData{ &CPU::JAM, &CPU::IMP };
        _opcodeTable[0x12] = InstructionData{ &CPU::JAM, &CPU::IMP };
        _opcodeTable[0x22] = InstructionData{ &CPU::JAM, &CPU::IMP };
        _opcodeTable[0x32] = InstructionData{ &CPU::JAM, &CPU::IMP };
        _opcodeTable[0x42] = InstructionData{ &CPU::JAM, &CPU::IMP };
        _opcodeTable[0x52] = InstructionData{ &CPU::JAM, &CPU::IMP };
        _opcodeTable[0x62] = InstructionData{ &CPU::JAM, &CPU::IMP };
        _opcodeTable[0x72] = InstructionData{ &CPU::JAM, &CPU::IMP };
        _opcodeTable[0x92] = InstructionData{ &CPU::JAM, &CPU::IMP };
        _opcodeTable[0xB2] = InstructionData{ &CPU::JAM, &CPU::IMP };
        _opcodeTable[0xD2] = InstructionData{ &CPU::JAM, &CPU::IMP };
        _opcodeTable[0xF2] = InstructionData{ &CPU::JAM, &CPU::IMP };

        // NOP Implied
        _opcodeTable[0x1A] = InstructionData{ &CPU::NOP, &CPU::IMP };
        _opcodeTable[0x3A] = InstructionData{ &CPU::NOP, &CPU::IMP };
        _opcodeTable[0x5A] = InstructionData{ &CPU::NOP, &CPU::IMP };
        _opcodeTable[0x7A] = InstructionData{ &CPU::NOP, &CPU::IMP };
        _opcodeTable[0xDA] = InstructionData{ &CPU::NOP, &CPU::IMP };
        _opcodeTable[0xFA] = InstructionData{ &CPU::NOP, &CPU::IMP };

        // NOP Immediate
        _opcodeTable[0x80] = InstructionData{ &CPU::NOP2, &CPU::IMM };
        _opcodeTable[0x82] = InstructionData{ &CPU::NOP2, &CPU::IMM };
        _opcodeTable[0x89] = InstructionData{ &CPU::NOP2, &CPU::IMM };
        _opcodeTable[0xC2] = InstructionData{ &CPU::NOP2, &CPU::IMM };
        _opcodeTable[0xE2] = InstructionData{ &CPU::NOP2, &CPU::IMM };

        // NOP Zero Page
        _opcodeTable[0x04] = InstructionData{ &CPU::NOP2, &CPU::ZPG };
        _opcodeTable[0x44] = InstructionData{ &CPU::NOP2, &CPU::ZPG };
        _opcodeTable[0x64] = InstructionData{ &CPU::NOP2, &CPU::ZPG };

        // NOP Zero Page X
        _opcodeTable[0x14] = InstructionData{ &CPU::NOP2, &CPU::ZPGX };
        _opcodeTable[0x34] = InstructionData{ &CPU::NOP2, &CPU::ZPGX };
        _opcodeTable[0x54] = InstructionData{ &CPU::NOP2, &CPU::ZPGX };
        _opcodeTable[0x74] = InstructionData{ &CPU::NOP2, &CPU::ZPGX };
        _opcodeTable[0xD4] = InstructionData{ &CPU::NOP2, &CPU::ZPGX };
        _opcodeTable[0xF4] = InstructionData{ &CPU::NOP2, &CPU::ZPGX };

        // NOP Absolute
        _opcodeTable[0x0C] = InstructionData{ &CPU::NOP2, &CPU::ABS };
        _opcodeTable[0x1C] = InstructionData{ &CPU::NOP2, &CPU::ABSX };
        _opcodeTable[0x3C] = InstructionData{ &CPU::NOP2, &CPU::ABSX };
        _opcodeTable[0x5C] = InstructionData{ &CPU::NOP2, &CPU::ABSX };
        _opcodeTable[0x7C] = InstructionData{ &CPU::NOP2, &CPU::ABSX };
        _opcodeTable[0xDC] = InstructionData{ &CPU::NOP2, &CPU::ABSX };
        _opcodeTable[0xFC] = InstructionData{ &CPU::NOP2, &CPU::ABSX };

        // SLO
        _opcodeTable[0x07] = InstructionData{ &CPU::SLO, &CPU::ZPG };
        _opcodeTable[0x17] = InstructionData{ &CPU::SLO, &CPU::ZPGX };
        _opcodeTable[0x0F] = InstructionData{ &CPU::SLO, &CPU::ABS };
        _opcodeTable[0x1F] = InstructionData{ &CPU::SLO, &CPU::ABSX, false, true };
        _opcodeTable[0x1B] = InstructionData{ &CPU::SLO, &CPU::ABSY, false, true };
        _opcodeTable[0x03] = InstructionData{ &CPU::SLO, &CPU::INDX };
        _opcodeTable[0x13] = InstructionData{ &CPU::SLO, &CPU::INDY, false, true };

        // RLA
        _opcodeTable[0x27] = InstructionData{ &CPU::RLA, &CPU::ZPG };
        _opcodeTable[0x37] = InstructionData{ &CPU::RLA, &CPU::ZPGX };
        _opcodeTable[0x2F] = InstructionData{ &CPU::RLA, &CPU::ABS };
        _opcodeTable[0x3F] = InstructionData{ &CPU::RLA, &CPU::ABSX, false, true };
        _opcodeTable[0x3B] = InstructionData{ &CPU::RLA, &CPU::ABSY, false, true };
        _opcodeTable[0x23] = InstructionData{ &CPU::RLA, &CPU::INDX };
        _opcodeTable[0x33] = InstructionData{ &CPU::RLA, &CPU::INDY, false, true };

        // SRE
        _opcodeTable[0x47] = InstructionData{ &CPU::SRE, &CPU::ZPG };
        _opcodeTable[0x57] = InstructionData{ &CPU::SRE, &CPU::ZPGX };
        _opcodeTable[0x4F] = InstructionData{ &CPU::SRE, &CPU::ABS };
        _opcodeTable[0x5F] = InstructionData{ &CPU::SRE, &CPU::ABSX, false, true };
        _opcodeTable[0x5B] = InstructionData{ &CPU::SRE, &CPU::ABSY, false, true };
        _opcodeTable[0x43] = InstructionData{ &CPU::SRE, &CPU::INDX };
        _opcodeTable[0x53] = InstructionData{ &CPU::SRE, &CPU::INDY, false, true };

        // RRA
        _opcodeTable[0x67] = InstructionData{ &CPU::RRA, &CPU::ZPG };
        _opcodeTable[0x77] = InstructionData{ &CPU::RRA, &CPU::ZPGX };
        _opcodeTable[0x6F] = InstructionData{ &CPU::RRA, &CPU::ABS };
        _opcodeTable[0x7F] = InstructionData{ &CPU::RRA, &CPU::ABSX, false, true };
        _opcodeTable[0x7B] = InstructionData{ &CPU::RRA, &CPU::ABSY, false, true };
        _opcodeTable[0x63] = InstructionData{ &CPU::RRA, &CPU::INDX };
        _opcodeTable[0x73] = InstructionData{ &CPU::RRA, &CPU::INDY, false, true };

        // SAX
        _opcodeTable[0x87] = InstructionData{ &CPU::SAX, &CPU::ZPG };
        _opcodeTable[0x97] = InstructionData{ &CPU::SAX, &CPU::ZPGY, true, true };
        _opcodeTable[0x8F] = InstructionData{ &CPU::SAX, &CPU::ABS };
        _opcodeTable[0x83] = InstructionData{ &CPU::SAX, &CPU::INDX };

        // LAX
        _opcodeTable[0xA7] = InstructionData{ &CPU::LAX, &CPU::ZPG };
        _opcodeTable[0xB7] = InstructionData{ &CPU::LAX, &CPU::ZPGY, true, true };
        _opcodeTable[0xAF] = InstructionData{ &CPU::LAX, &CPU::ABS };
        _opcodeTable[0xBF] = InstructionData{ &CPU::LAX, &CPU::ABSY };
        _opcodeTable[0xA3] = InstructionData{ &CPU::LAX, &CPU::INDX };
        _opcodeTable[0xB3] = InstructionData{ &CPU::LAX, &CPU::INDY };

        // DCP
        _opcodeTable[0xC7] = InstructionData{ &CPU::DCP, &CPU::ZPG };
        _opcodeTable[0xD7] = InstructionData{ &CPU::DCP, &CPU::ZPGX };
        _opcodeTable[0xCF] = InstructionData{ &CPU::DCP, &CPU::ABS };
        _opcodeTable[0xDF] = InstructionData{ &CPU::DCP, &CPU::ABSX, false, true };
        _opcodeTable[0xDB] = InstructionData{ &CPU::DCP, &CPU::ABSY, false, true };
        _opcodeTable[0xC3] = InstructionData{ &CPU::DCP, &CPU::INDX };
        _opcodeTable[0xD3] = InstructionData{ &CPU::DCP, &CPU::INDY, false, true };

        // ISC
        _opcodeTable[0xE7] = InstructionData{ &CPU::ISC, &CPU::ZPG };
        _opcodeTable[0xF7] = InstructionData{ &CPU::ISC, &CPU::ZPGX };
        _opcodeTable[0xEF] = InstructionData{ &CPU::ISC, &CPU::ABS };
        _opcodeTable[0xFF] = InstructionData{ &CPU::ISC, &CPU::ABSX, false, true };
        _opcodeTable[0xFB] = InstructionData{ &CPU::ISC, &CPU::ABSY, false, true };
        _opcodeTable[0xE3] = InstructionData{ &CPU::ISC, &CPU::INDX };
        _opcodeTable[0xF3] = InstructionData{ &CPU::ISC, &CPU::INDY, false, true };

        // SBC2
        _opcodeTable[0xEB] = InstructionData{ &CPU::SBC, &CPU::IMM };

        // ALR, ARR
        _opcodeTable[0x4B] = InstructionData{ &CPU::ALR, &CPU::IMM };
        _opcodeTable[0x6B] = InstructionData{ &CPU::ARR, &CPU::IMM };

        // ANC
        _opcodeTable[0x0B] = InstructionData{ &CPU::ANC, &CPU::IMM };
        _opcodeTable[0x2B] = InstructionData{ &CPU::ANC, &CPU::IMM };

        // LXA
        _opcodeTable[0xAB] = InstructionData{ &CPU::LXA, &CPU::IMM };

        // SBX
        _opcodeTable[0xCB] = InstructionData{ &CPU::SBX, &CPU::IMM };

        // LAS
        _opcodeTable[0xBB] = InstructionData{ &CPU::LAS, &CPU::ABSY };

        // ANE
        _opcodeTable[0x8B] = InstructionData{ &CPU::ANE, &CPU::IMM };
    }

    /*
    ################################
    ||           Getters          ||
    ################################
    */
    u8   GetAccumulator() const { return _a; }
    u8   GetXRegister() const { return _x; }
    u8   GetYRegister() const { return _y; }
    u8   GetStatusRegister() const { return _p; }
    u16  GetProgramCounter() const { return _pc; }
    u8   GetStackPointer() const { return _s; }
    u64  GetCycles() const { return _cycles; }
    bool IsReading2002() const { return _reading2002; }
    bool IsNmiInProgress() const { return _nmiInProgress; }

    // status getters
    u8 GetCarryFlag() const { return ( _p & Carry ) >> 0; }
    u8 GetZeroFlag() const { return ( _p & Zero ) >> 1; }
    u8 GetInterruptDisableFlag() const { return ( _p & InterruptDisable ) >> 2; }
    u8 GetDecimalFlag() const { return ( _p & Decimal ) >> 3; }
    u8 GetBreakFlag() const { return ( _p & Break ) >> 4; }
    u8 GetOverflowFlag() const { return ( _p & Overflow ) >> 6; }
    u8 GetNegativeFlag() const { return ( _p & Negative ) >> 7; }

    /*
    ################################
    ||           Setters          ||
    ################################
    */
    void SetAccumulator( u8 value ) { _a = value; }
    void SetXRegister( u8 value ) { _x = value; }
    void SetYRegister( u8 value ) { _y = value; }
    void SetStatusRegister( u8 value ) { _p = value; }
    void SetProgramCounter( u16 value ) { _pc = value; }
    void SetStackPointer( u8 value ) { _s = value; }
    void SetCycles( u64 value ) { _cycles = value; }
    void SetReading2002( bool value ) { _reading2002 = value; };
    void SetNmiInProgress( bool value ) { _nmiInProgress = value; }

    // status setters
    void SetCarryFlag( bool value ) { value ? SetFlags( Carry ) : ClearFlags( Carry ); }
    void SetZeroFlag( bool value ) { value ? SetFlags( Zero ) : ClearFlags( Zero ); }
    void SetInterruptDisableFlag( bool value )
    {
        value ? SetFlags( InterruptDisable ) : ClearFlags( InterruptDisable );
    }
    void SetDecimalFlag( bool value ) { value ? SetFlags( Decimal ) : ClearFlags( Decimal ); }
    void SetBreakFlag( bool value ) { value ? SetFlags( Break ) : ClearFlags( Break ); }
    void SetOverflowFlag( bool value ) { value ? SetFlags( Overflow ) : ClearFlags( Overflow ); }
    void SetNegativeFlag( bool value ) { value ? SetFlags( Negative ) : ClearFlags( Negative ); }

    /*
    ################################
    ||         CPU Methods        ||
    ################################
    */
    void Reset();
    u8   Fetch();
    void DecodeExecute();
    void Tick();
    auto Read( u16 address, bool debugMode = false ) const -> u8;
    auto ReadAndTick( u16 address ) -> u8;
    void Write( u16 address, u8 data ) const;
    void WriteAndTick( u16 address, u8 data );

    void NMI()
    {
        /* @details: Non-maskable Interrupt, called by the PPU during the VBlank period.
         * It interrupts whatever the CPU is doing at its current cycle to go update the PPU.
         * Uses 7 cycles, cannot be disabled.
         */
        SetNmiInProgress( true );
        // 1) Two dummy cycles (hardware reads the same PC twice, discarding the data)
        Tick();
        Tick();

        // 2) Push PC high, then PC low
        StackPush( ( _pc >> 8 ) & 0xFF );
        StackPush( _pc & 0xFF );

        // 3) Push status register with B=0; bit 5 (Unused) = 1
        u8 const pushedStatus = ( _p & ~Break ) | Unused;
        StackPush( pushedStatus );

        // 4) Fetch low byte of NMI vector ($FFFA)
        u8 const low = ReadAndTick( 0xFFFA );

        // 5) Set I flag
        SetFlags( InterruptDisable );

        // 6) Fetch high byte of NMI vector ($FFFB)
        u8 const high = ReadAndTick( 0xFFFB );

        // 7) Update PC
        _pc = static_cast<u16>( high ) << 8 | low;

        SetNmiInProgress( false );
    }

    void IRQ()
    {
        /* @brief: IRQ, can be called when interrupt disable is turned off.
         * Uses 7 cycles
         */
        if ( ( _p & InterruptDisable ) != 0 ) {
            return;
        }
        Tick();
        Tick();
        StackPush( ( _pc >> 8 ) & 0xFF );
        StackPush( _pc & 0xFF );
        u8 const pushedStatus = ( _p & ~Break ) | Unused;
        StackPush( pushedStatus );
        u8 const low = ReadAndTick( 0xFFFE );
        SetFlags( InterruptDisable );
        u8 const high = ReadAndTick( 0xFFFF );
        _pc = static_cast<u16>( high ) << 8 | low;
    }

    /*
    ################################
    ||        Debug Methods       ||
    ################################
    */
    std::string             LogLineAtPC( bool verbose = true );
    std::deque<std::string> GetTracelog() const { return _traceLog; }
    std::deque<std::string> GetMesenFormatTracelog() const { return _mesenFormatTraceLog; }
    void                    EnableTracelog()
    {
        _traceEnabled = true;
        _mesenFormatTraceEnabled = false;
    }
    void EnableMesenFormatTraceLog()
    {
        _mesenFormatTraceEnabled = true;
        _traceEnabled = false;
    }
    void DisableTracelog() { _traceEnabled = false; }
    void DisableMesenFormatTraceLog() { _mesenFormatTraceEnabled = false; }
    void EnableJsonTestMode() { _isTestMode = true; }
    void DisableJsonTestMode() { _isTestMode = false; }

    u16  traceSize = 100;
    u16  mesenTraceSize = 100;
    void AddTraceLog( const std::string &log )
    {
        if ( _traceEnabled ) {
            _traceLog.push_back( log + "\n" );
            if ( _traceLog.size() > traceSize ) {
                _traceLog.pop_front();
            }
        }
    }
    void ClearTraceLog() { _traceLog.clear(); }
    void AddMesenTracelog( const std::string &log )
    {
        if ( _mesenFormatTraceEnabled ) {
            _mesenFormatTraceLog.push_back( log + "\n" );
            if ( _mesenFormatTraceLog.size() > mesenTraceSize ) {
                _mesenFormatTraceLog.pop_front();
            }
        }
    }
    void ClearMesenTraceLog() { _mesenFormatTraceLog.clear(); }

    /*
    ################################
    ||      Global Variables      ||
    ################################
    */
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

  private:
    friend class CPUTestFixture; // Sometimes used for testing private methods

    /*
    ################################
    ||          Registers         ||
    ################################
    */
    u16 _pc = 0x0000;       // Program counter (PC)
    u8  _a = 0x00;          // Accumulator register (A)
    u8  _x = 0x00;          // X register
    u8  _y = 0x00;          // Y register
    u8  _s = 0xFD;          // Stack pointer (SP)
    u8  _p = 0x00 | Unused; // Status register (P), per the specs, the unused flag should always be set
    u64 _cycles = 0;        // Number of cycles

    /*
    ################################
    ||  Private Global Variables  ||
    ################################
    */
    bool        _didVblank = false;
    bool        _currentPageCrossPenalty = true;
    bool        _isWriteModify = false;
    bool        _reading2002 = false;
    bool        _nmiInProgress = false;
    std::string _instructionName;
    std::string _addrMode;

    /*
    ################################
    ||       Debug Variables      ||
    ################################
    */
    bool _isTestMode = false;
    bool _traceEnabled = false;
    bool _mesenFormatTraceEnabled = false;
    bool _didMesenTrace = false;

    std::deque<std::string> _traceLog;
    std::deque<std::string> _mesenFormatTraceLog;

    /*
    ################################
    ||         Peripherals        ||
    ################################
    */
    Bus *_bus;

    /*
    ################################
    ||        Opcode Table        ||
    ################################
    */
    struct InstructionData {
        void ( CPU::*instructionMethod )( u16 ){}; // Pointer to the instruction helper method
        u16 ( CPU::*addressingModeMethod )(){};    // Pointer to the address mode helper method
        bool pageCrossPenalty =
            true; // Whether the instruction takes an extra cycle if a page boundary is crossed
        bool isWriteModify = false; // Write/modify instructions use a dummy read before writing
    };

    // Opcode table
    std::array<InstructionData, 256> _opcodeTable;
    InstructionData                  GetInstruction( u8 opcode ) { return _opcodeTable[opcode]; }

    /*
    ################################################################
    ||                                                            ||
    ||                    Instruction Helpers                     ||
    ||                                                            ||
    ################################################################
    */

    // Flag methods
    void SetFlags( u8 flag );
    void ClearFlags( u8 flag );
    auto IsFlagSet( u8 flag ) const -> bool;
    void SetZeroAndNegativeFlags( u8 value );

    // LDA, LDX, and LDY helper
    void LoadRegister( u16 address, u8 &reg );
    void StoreRegister( u16 address, u8 reg );

    // Branch helper
    void BranchOnStatus( u16 offsetAddress, u8 flag, bool isSet );

    // Compare helper
    void CompareAddressWithRegister( u16 address, u8 reg );

    // Push/Pop helper
    void StackPush( u8 value );
    auto StackPop() -> u8;

    /*
    ################################################################
    ||                                                            ||
    ||                      Addressing Modes                      ||
    ||                                                            ||
    ################################################################
    */
    auto IMP() -> u16;  // Implicit
    auto IMM() -> u16;  // Immediate
    auto ZPG() -> u16;  // Zero Page
    auto ZPGX() -> u16; // Zero Page X
    auto ZPGY() -> u16; // Zero Page Y
    auto ABS() -> u16;  // Absolute
    auto ABSX() -> u16; // Absolute X
    auto ABSY() -> u16; // Absolute Y
    auto IND() -> u16;  // Indirect
    auto INDX() -> u16; // Indirect X
    auto INDY() -> u16; // Indirect Y
    auto REL() -> u16;  // Relative
    /*
    ################################################################
    ||                                                            ||
    ||                        Instructions                        ||
    ||                                                            ||
    ################################################################
      */

    // NOP
    void NOP( u16 address );

    // Load/Store
    void LDA( u16 address );
    void LDX( u16 address );
    void LDY( u16 address );
    void STA( u16 address );
    void STX( u16 address );
    void STY( u16 address );

    // Arithmetic
    void ADC( u16 address );
    void SBC( u16 address );
    void INC( u16 address );
    void DEC( u16 address );
    void INX( u16 address );
    void INY( u16 address );
    void DEX( u16 address );
    void DEY( u16 address );

    // Clear/Set flags
    void CLC( u16 address );
    void CLI( u16 address );
    void CLD( u16 address );
    void CLV( u16 address );
    void SEC( u16 address );
    void SED( u16 address );
    void SEI( u16 address );

    // Branch
    void BPL( u16 address );
    void BMI( u16 address );
    void BVC( u16 address );
    void BVS( u16 address );
    void BCC( u16 address );
    void BCS( u16 address );
    void BNE( u16 address );
    void BEQ( u16 address );

    // Compare
    void CMP( u16 address );
    void CPX( u16 address );
    void CPY( u16 address );

    // Shift
    void ASL( u16 address );
    void LSR( u16 address );
    void ROL( u16 address );
    void ROR( u16 address );

    // Stack
    void PHA( u16 address );
    void PHP( u16 address );
    void PLA( u16 address );
    void PLP( u16 address );
    void TSX( u16 address );
    void TXS( u16 address );

    // Jumps
    void JMP( u16 address );
    void JSR( u16 address );
    void RTS( u16 address );
    void RTI( u16 address );
    void BRK( u16 address );

    // Bitwise
    void AND( u16 address );
    void EOR( u16 address );
    void ORA( u16 address );
    void BIT( u16 address );

    // Transfer
    void TAX( u16 address );
    void TXA( u16 address );
    void TAY( u16 address );
    void TYA( u16 address );

    /*
    ################################################
    ||                                            ||
    ||               Illegal Opcodes              ||
    ||                                            ||
    ################################################
    */
    void NOP2( u16 address );
    void JAM( u16 address );
    void SLO( u16 address );
    void SAX( u16 address );
    void LXA( u16 address );
    void LAX( u16 address );
    void ARR( u16 address );
    void ALR( u16 address );
    void RRA( u16 address );
    void SRE( u16 address );
    void RLA( u16 address );
    void DCP( u16 address );
    void ISC( u16 address );
    void ANC( u16 address );
    void SBX( u16 address );
    void LAS( u16 address );
    void ANE( u16 address );
};
