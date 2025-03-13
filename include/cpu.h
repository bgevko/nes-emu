#pragma once

#include <array>
#include <deque>
#include <cstdint>
#include <fmt/base.h>
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
          //0      1        2        3        4        5        6        7        8        9        A       B        C        D        E        F
    /*0*/"BRK",   "ORA",   "*JAM",  "*SLO",  "*NOP",  "ORA",   "*ASL",  "*SLO",  "PHP",  "ORA",   "ASL",   "*ANC",  "*NOP",  "ORA",   "ASL",   "*SLO",
    /*1*/"BPL",   "ORA",   "*JAM",  "*SLO",  "*NOP",  "ORA",   "*ASL",  "*SLO",  "CLC",  "ORA",   "*NOP",  "*SLO",  "*NOP",  "ORA",   "ASL",   "*SLO",
    /*2*/"JSR",   "AND",   "*JAM",  "*RLA",  "BIT",   "AND",   "ROL",   "*RLA",  "PLP",  "AND",   "ROL",   "*ANC",  "BIT",   "AND",   "ROL",   "*RLA",
    /*3*/"BMI",   "AND",   "*JAM",  "*RLA",  "*NOP",  "AND",   "*ROL",  "*RLA",  "SEC",  "AND",   "*NOP",  "*RLA",  "*NOP",  "AND",   "ROL",   "*RLA",
    /*4*/"RTI",   "EOR",   "*JAM",  "*SRE",  "*NOP",  "EOR",   "*LSR",  "*SRE",  "PHA",  "EOR",   "LSR",   "*ALR",  "JMP",   "EOR",   "LSR",   "*SRE",
    /*5*/"BVC",   "EOR",   "*JAM",  "*SRE",  "*NOP",  "EOR",   "*LSR",  "*SRE",  "CLI",  "EOR",   "*NOP",  "*SRE",  "*NOP",  "EOR",   "LSR",   "*SRE",
    /*6*/"RTS",   "ADC",   "*JAM",  "*RRA",  "*NOP",  "ADC",   "*ROR",  "*RRA",  "PLA",  "ADC",   "ROR",   "*ARR",  "JMP",   "ADC",   "ROR",   "*RRA",
    /*7*/"BVS",   "ADC",   "*JAM",  "*RRA",  "*NOP",  "ADC",   "*ROR",  "*RRA",  "SEI",  "ADC",   "*NOP",  "*RRA",  "*NOP",  "ADC",   "ROR",   "*RRA",
    /*8*/"*NOP",  "STA",   "*NOP",  "*SAX",  "STY",   "STA",   "STX",   "*SAX",  "DEY",  "*NOP",  "TXA",   "*ANE",  "STY",   "STA",   "STX",   "*SAX",
    /*9*/"BCC",   "STA",   "*JAM",  "*SHA",  "STY",   "STA",   "STX",   "*STX",  "TYA",  "STA",   "TXS",   "*TAS",  "*SHY",  "STA",   "*SHX",  "*SHA",
    /*A*/"LDY",   "LDA",   "LDX",   "*LAX",  "LDY",   "LDA",   "LDX",   "*LAX",  "TAY",  "LDA",   "TAX",   "*LXA",  "LDY",   "LDA",   "LDX",   "*LAX",
    /*B*/"BCS",   "LDA",   "*JAM",  "*LAX",  "LDY",   "LDA",   "LDX",   "*LAX",  "CLV",  "LDA",   "TSX",   "*LAS",  "LDY",   "LDA",   "LDX",   "*LAX",
    /*C*/"CPY",   "CMP",   "*NOP",  "*DCP",  "CPY",   "CMP",   "DEC",   "*DCP",  "INY",  "CMP",   "DEX",   "*SBX",  "CPY",   "CMP",   "DEC",   "*DCP",
    /*D*/"BNE",   "CMP",   "*JAM",  "*DCP",  "*NOP",  "CMP",   "*DEC",  "*DCP",  "CLD",  "CMP",   "*NOP",  "*DCP",  "*NOP",  "CMP",   "DEC",   "*DCP",
    /*E*/"CPX",   "SBC",   "*NOP",  "*ISC",  "CPX",   "SBC",   "INC",   "*ISC",  "INX",  "SBC",   "NOP",   "*SBC",  "CPX",   "SBC",   "INC",   "*ISC",
    /*F*/"BEQ",   "SBC",   "*JAM",  "*ISC",  "*NOP",  "SBC",   "*INC",  "*ISC",  "SED",  "SBC",   "*NOP",  "*ISC",  "*NOP",  "SBC",   "INC",   "*ISC"
};

const std::array<std::string, 256> gAddressingModes = {
          //0      1        2        3        4        5        6        7        8        9        A       B        C        D        E        F
     /*0*/"IMP",   "INDX",  "IMP",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
     /*1*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX",
     /*2*/"ABS",   "INDX",  "IMP",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
     /*3*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX",
     /*4*/"IMP",   "INDX",  "IMP",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
     /*5*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX",
     /*6*/"IMP",   "INDX",  "IMP",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "IND",   "ABS",   "ABS",   "ABS",
     /*7*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX",
     /*8*/"IMM",   "INDX",  "IMM",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
     /*9*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGY",  "ZPGY",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSY",  "ABSY",
     /*A*/"IMM",   "INDX",  "IMM",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
     /*B*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGY",  "ZPGY",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSY",  "ABSY",
     /*C*/"IMM",   "INDX",  "IMM",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
     /*D*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX",
     /*E*/"IMM",   "INDX",  "IMM",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
     /*F*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX"
};

const std::array<u8, 256> gInstructionCycles = {
          //0      1        2        3        4        5        6        7        8        9        A       B        C        D        E        F
     /*0*/7,       6,       2,       8,       3,       3,       5,       5,       3,       2,       2,      2,       4,       4,       6,       6,
     /*1*/2,       5,       2,       8,       4,       4,       6,       6,       2,       4,       2,      7,       4,       4,       7,       7,
     /*2*/6,       6,       2,       8,       3,       3,       5,       5,       4,       2,       2,      2,       4,       4,       6,       6,
     /*3*/2,       5,       2,       8,       4,       4,       6,       6,       2,       4,       2,      7,       4,       4,       7,       7,
     /*4*/6,       6,       2,       8,       3,       3,       5,       5,       3,       2,       2,      2,       3,       4,       6,       6,
     /*5*/2,       5,       2,       8,       4,       4,       6,       6,       2,       4,       2,      7,       4,       4,       7,       7,
     /*6*/6,       6,       2,       8,       3,       3,       5,       5,       4,       2,       2,      2,       5,       4,       6,       6,
     /*7*/2,       5,       2,       8,       4,       4,       6,       6,       2,       4,       2,      7,       4,       4,       7,       7,
     /*8*/2,       6,       2,       6,       3,       3,       3,       3,       2,       2,       2,      2,       4,       4,       4,       4,
     /*9*/2,       6,       2,       6,       4,       4,       4,       4,       2,       5,       2,      5,       5,       5,       5,       5,
     /*A*/2,       6,       2,       6,       3,       3,       3,       3,       2,       2,       2,      2,       4,       4,       4,       4,
     /*B*/2,       5,       2,       5,       4,       4,       4,       4,       2,       4,       2,      4,       4,       4,       4,       4,
     /*C*/2,       6,       2,       8,       3,       3,       5,       5,       2,       2,       2,      2,       4,       4,       6,       6,
     /*D*/2,       5,       2,       8,       4,       4,       6,       6,       2,       4,       2,      7,       4,       4,       7,       7,
     /*E*/2,       6,       2,       8,       3,       3,       5,       5,       2,       2,       2,      2,       4,       4,       6,       6,
     /*F*/2,       5,       2,       8,       4,       4,       6,       6,       2,       4,       2,      7,       4,       4,       7,       7
};

const std::array<u8, 256> gInstructionBytes = {
          //0      1        2        3        4        5        6        7        8        9        A       B        C        D        E        F
     /*0*/1,       2,       1,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*1*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3,
     /*2*/3,       2,       1,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*3*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3,
     /*4*/1,       2,       1,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*5*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3,
     /*6*/1,       2,       1,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*7*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3,
     /*8*/2,       2,       2,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*9*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3,
     /*A*/2,       2,       2,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*B*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3,
     /*C*/2,       2,       2,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*D*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3,
     /*E*/2,       2,       2,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*F*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3
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

    size_t traceSize = 100;
    size_t mesenTraceSize = 100;
    void   SetMesenTraceSize( int size ) { mesenTraceSize = size; }
    void   AddTraceLog( const std::string &log )
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

    void LoadRegister( u16 address, u8 &reg )
    {
        /*
         * @brief It loads a register with a value from memory
         * Used by LDA, LDX, and LDY instructions
         */
        u8 const value = ReadAndTick( address );
        reg = value;

        // Set zero and negative flags
        SetZeroAndNegativeFlags( value );
    };

    void StoreRegister( u16 address, u8 reg )
    {
        /*
         * @brief It stores a register value in memory
         * Used by STA, STX, and STY instructions
         */
        WriteAndTick( address, reg );
    };

    void SetFlags( const u8 flag )
    {
        /*
         * @brief set one or more flag bits through bitwise OR with the status register
         *
         * Used by the SEC, SED, and SEI instructions to set one or more flag bits through
         * bitwise OR with the status register.
         *
         * Usage:
         * SetFlags( Status::Carry ); // Set one flag
         * SetFlags( Status::Carry | Status::Zero ); // Set multiple flags
         */
        _p |= flag;
    }
    void ClearFlags( const u8 flag )
    {
        /* Clear Flags
         * @brief clear one or more flag bits through bitwise AND of the complement (inverted) flag
         * with the status register
         *
         * Used by the CLC, CLD, and CLI instructions to clear one or more flag bits through
         * bitwise AND of the complement (inverted) flag with the status register.
         *
         * Usage:
         * ClearFlags( Status::Carry ); // Clear one flag
         * ClearFlags( Status::Carry | Status::Zero ); // Clear multiple flags
         */
        _p &= ~flag;
    }
    bool IsFlagSet( const u8 flag ) const
    {
        /* @brief Utility function to check if a given status is set in the status register
         *
         * Usage:
         * if ( IsFlagSet( Status::Carry ) )
         * {
         *   // Do something
         * }
         * if ( IsFlagSet( Status::Carry | Status::Zero ) )
         * {
         *   // Do something
         * }
         */
        return ( _p & flag ) == flag;
    }

    void SetZeroAndNegativeFlags( u8 value )
    {
        /*
         * @brief Sets zero flag if value == 0, or negative flag if value is negative (bit 7 is set)
         */

        // Clear zero and negative flags
        ClearFlags( Status::Zero | Status::Negative );

        // Set zero flag if value is zero
        if ( value == 0 ) {
            SetFlags( Status::Zero );
        }

        // Set negative flag if bit 7 is set
        if ( ( value & 0b10000000 ) != 0 ) {
            SetFlags( Status::Negative );
        }
    }

    void BranchOnStatus( u16 offsetAddress, u8 flag, bool isSet )
    {
        /* @brief Branch if status flag is set or clear
         *
         * Used by branch instructions to branch if a status flag is set or clear.
         *
         * Usage:
         * BranchOnStatus( Status::Carry, true ); // Branch if carry flag is set
         * BranchOnStatus( Status::Zero, false ); // Branch if zero flag is clear
         */

        bool const willBranch = ( _p & flag ) == flag;

        // Path will branch
        if ( willBranch == isSet ) {
            // Store previous program counter value, used to check boundary crossing
            u16 const prevPc = _pc;

            // Set _pc to the offset address, calculated by REL addressing mode
            _pc = offsetAddress;

            // +1 cycles because we're taking a branch
            Tick();

            // Add another cycle if page boundary is crossed
            if ( ( _pc & 0xFF00 ) != ( prevPc & 0xFF00 ) ) {
                Tick();
            }
        }
        // Path will not branch, nothing to do
    }

    void CompareAddressWithRegister( u16 address, u8 reg )
    {
        /*
         * @brief Compare a value in memory with a register
         * Used by CMP, CPX, and CPY instructions
         */

        u8 value = 0;
        if ( _instructionName == "*DCP" ) {
            value = Read( address ); // 0 cycles
        } else {
            value = ReadAndTick( address );
        }

        // Set the zero flag if the values are equal
        ( reg == value ) ? SetFlags( Status::Zero ) : ClearFlags( Status::Zero );

        // Set negative flag if the result is negative,
        // i.e. the sign bit is set
        ( ( reg - value ) & 0b10000000 ) != 0 ? SetFlags( Status::Negative ) : ClearFlags( Status::Negative );

        // Set the carry flag if the reg >= value
        ( reg >= value ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );
    }

    void StackPush( u8 value )
    {
        /*
         * @brief Push a value onto the stack
         * The stack pointer is decremented and the value is written to the stack
         * Stack addresses are between 0x0100 and 0x01FF
         */
        WriteAndTick( 0x0100 + _s--, value );
    }

    u8 StackPop()
    {
        /*
         * @brief Pop a value from the stack
         * The stack pointer is incremented and the value is read from the stack
         * Stack addresses are between 0x0100 and 0x01FF
         */
        return ReadAndTick( 0x0100 + ++_s );
    }

    /*
    ################################################################
    ||                                                            ||
    ||                      Addressing Modes                      ||
    ||                                                            ||
    ################################################################
    */

    auto IMP() -> u16
    {
        /*
         * @brief Implicit addressing mode
         * This mode does not require an operand
         */
        Tick();
        return 0;
    }

    auto IMM() -> u16
    {
        /*
         * @brief Returns address of the next byte in memory (the operand itself)
         * The operand is a part of the instruction
         * The program counter is incremented to point to the operand
         */
        return _pc++;
    }

    auto ZPG() -> u16
    {
        /*
         * @brief Zero Page addressing mode
         * Returns the address from the zero page (0x0000 - 0x00FF).
         * The value of the next byte is the address in the zero page.
         */
        return ReadAndTick( _pc++ ) & 0x00FF;
    }

    auto ZPGX() -> u16
    {
        /*
         * @brief Zero Page X addressing mode
         * Returns the address from the zero page (0x0000 - 0x00FF) + X register
         * The value of the next byte is the address in the zero page.
         */
        u8 const  zeroPageAddress = ReadAndTick( _pc++ );
        u16 const finalAddress = ( zeroPageAddress + _x ) & 0x00FF;
        Tick(); // Account for calculating the final address
        return finalAddress;
    }

    auto ZPGY() -> u16
    {
        /*
         * @brief Zero Page Y addressing mode
         * Returns the address from the zero page (0x0000 - 0x00FF) + Y register
         * The value of the next byte is the address in the zero page.
         */
        u8 const zeroPageAddress = ( ReadAndTick( _pc++ ) + _y ) & 0x00FF;

        if ( _isWriteModify ) {
            Tick();
        }
        return zeroPageAddress;
    }

    auto ABS() -> u16
    {
        /*
         * @brief Absolute addressing mode
         * Constructs a 16-bit address from the next two bytes
         */
        u16 const low = ReadAndTick( _pc++ );
        u16 const high = ReadAndTick( _pc++ );
        return ( high << 8 ) | low;
    }

    auto ABSX() -> u16
    {
        /*
         * @brief Absolute X addressing mode
         * Constructs a 16-bit address from the next two bytes and adds the X register to the final
         * address
         */
        u16 const low = ReadAndTick( _pc++ );
        u16 const high = ReadAndTick( _pc++ );
        u16 const address = ( high << 8 ) | low;
        u16 const finalAddress = address + _x;

        // If the final address crosses a page boundary, an additional cycle is required
        // Instructions that should ignore this: ASL, ROL, LSR, ROR, STA, DEC, INC
        if ( _currentPageCrossPenalty && ( finalAddress & 0xFF00 ) != ( address & 0xFF00 ) ) {
            Tick();
        }

        if ( _isWriteModify ) {
            // Dummy read, in preparation to overwrite the address
            Tick();
        }
        return finalAddress;
    }

    auto ABSY() -> u16
    {
        /*
         * @brief Absolute Y addressing mode
         * Constructs a 16-bit address from the next two bytes and adds the Y register to the final
         * address
         */
        u16 const low = ReadAndTick( _pc++ );
        u16 const high = ReadAndTick( _pc++ );
        u16 const address = ( high << 8 ) | low;
        u16 const finalAddress = address + _y;

        // If the final address crosses a page boundary, an additional cycle is required
        // Instructions that should ignore this: STA
        if ( _currentPageCrossPenalty && ( finalAddress & 0xFF00 ) != ( address & 0xFF00 ) ) {
            Tick();
        }
        if ( _isWriteModify ) {
            // Dummy read, in preparation to overwrite the address
            Tick();
        }

        return finalAddress;
    }

    auto IND() -> u16
    {
        /*
         * @brief Indirect addressing mode
         * This mode implements pointers.
         * The pointer address will be read from the next two bytes
         * The returning value is the address stored at the pointer address
         * There's a hardware bug that prevents the address from crossing a page boundary
         */

        u16 const ptrLow = ReadAndTick( _pc++ );
        u16 const ptrHigh = ReadAndTick( _pc++ );
        u16 const ptr = ( ptrHigh << 8 ) | ptrLow;

        u8 const addressLow = ReadAndTick( ptr );
        u8       address_high; // NOLINT

        // 6502 Bug: If the pointer address wraps around a page boundary (e.g. 0x01FF),
        // the CPU reads the low byte from 0x01FF and the high byte from the start of
        // the same page (0x0100) instead of the start of the next page (0x0200).
        if ( ptrLow == 0xFF ) {
            address_high = ReadAndTick( ptr & 0xFF00 );
        } else {
            address_high = ReadAndTick( ptr + 1 );
        }

        return ( address_high << 8 ) | addressLow;
    }

    auto INDX() -> u16
    {
        /*
         * @brief Indirect X addressing mode
         * The next two bytes are a zero-page address
         * X register is added to the zero-page address to get the pointer address
         * Final address is the value stored at the POINTER address
         */
        Tick();                                                              // Account for operand fetch
        u8 const  zeroPageAddress = ( ReadAndTick( _pc++ ) + _x ) & 0x00FF;  // 1 cycle
        u16 const ptrLow = ReadAndTick( zeroPageAddress );                   // 1 cycle
        u16 const ptrHigh = ReadAndTick( ( zeroPageAddress + 1 ) & 0x00FF ); // 1 cycle
        return ( ptrHigh << 8 ) | ptrLow;
    }

    auto INDY() -> u16
    {
        /*
         * @brief Indirect Y addressing mode
         * The next byte is a zero-page address
         * The value stored at the zero-page address is the pointer address
         * The value in the Y register is added to the FINAL address
         */
        u16 const zeroPageAddress = ReadAndTick( _pc++ );
        u16 const ptrLow = ReadAndTick( zeroPageAddress );
        u16 const ptrHigh = ReadAndTick( ( zeroPageAddress + 1 ) & 0x00FF );

        u16 const address = ( ( ptrHigh << 8 ) | ptrLow ) + _y;

        // If the final address crosses a page boundary, an additional cycle is required
        // Instructions that should ignore this: STA
        if ( _currentPageCrossPenalty && ( address & 0xFF00 ) != ( ptrHigh << 8 ) ) {
            Tick();
        }

        if ( _isWriteModify ) {
            // Dummy read, in preparation to overwrite the address
            Tick();
        }
        return address;
    }

    auto REL() -> u16
    {
        /*
         * @brief Relative addressing mode
         * The next byte is a signed offset
         * Sets the program counter between -128 and +127 bytes from the current location
         */
        s8 const  offset = static_cast<s8>( ReadAndTick( _pc++ ) );
        u16 const address = _pc + offset;
        return address;
    }

    /*
    ################################################################
    ||                                                            ||
    ||                        Instructions                        ||
    ||                                                            ||
    ################################################################
      */

    void NOP( u16 address ) // NOLINT
    {
        /*
         * @brief No operation
         * N Z C I D V
         * - - - - - -
         * Usage and cycles:
         * NOP Implied: EA(2)
         *
         * --  Illegal  --
         * NOP Implied: 1A(2)
         * NOP Implied: 3A(2)
         * NOP Implied: 5A(2)
         * NOP Implied: 7A(2)
         * NOP Implied: DA(2)
         * NOP Implied: FA(2)
         * NOP Immediate: 80(2)
         * NOP Immediate: 82(2)
         * NOP Immediate: 89(2)
         * NOP Immediate: C2(2)
         * NOP Immediate: E2(2)
         * NOP Zero Page: 04(3)
         * NOP Zero Page: 44(3)
         * NOP Zero Page: 64(3)
         * NOP Zero Page X: 14(4)
         * NOP Zero Page X: 34(4)
         * NOP Zero Page X: 54(4)
         * NOP Zero Page X: 74(4)
         * NOP Zero Page X: D4(4)
         * NOP Zero Page X: F4(4)
         * NOP Absolute: 0C(4)
         * NOP Absolute: 1C(4)
         * NOP Absolute: 3C(4)
         * NOP Absolute: 5C(4)
         * NOP Absolute: 7C(4)
         * NOP Absolute: DC(4)
         * NOP Absolute: FC(4)
         */
        (void) address;
    }

    void LDA( u16 address )
    {
        /*
         * @brief Load Accumulator with Memory
         * N Z C I D V
         * + + - - - -
         * Usage and cycles:
         * LDA Immediate: A9(2)
         * LDA Zero Page: A5(3)
         * LDA Zero Page X: B5(4)
         * LDA Absolute: AD(4)
         * LDA Absolute X: BD(4+)
         * LDA Absolute Y: B9(4+)
         * LDA Indirect X: A1(6)
         * LDA Indirect Y: B1(5+)
         */

        LoadRegister( address, _a );
    }

    void LDX( u16 address )
    {
        /*
         * @brief Load X Register with Memory
         * N Z C I D V
         * + + - - - -
         * Usage and cycles:
         * LDX Immediate: A2(2)
         * LDX Zero Page: A6(3)
         * LDX Zero Page Y: B6(4)
         * LDX Absolute: AE(4)
         * LDX Absolute Y: BE(4+)
         */
        LoadRegister( address, _x );
    }

    void LDY( u16 address )
    {
        /*
         * @brief Load Y Register with Memory
         * N Z C I D V
         * + + - - - -
         * Usage and cycles:
         * LDY Immediate: A0(2)
         * LDY Zero Page: A4(3)
         * LDY Zero Page X: B4(4)
         * LDY Absolute: AC(4)
         * LDY Absolute X: BC(4+)
         */
        LoadRegister( address, _y );
    }

    void STA( const u16 address ) // NOLINT
    {
        /*
         * @brief Store Accumulator in Memory
         * N Z C I D V
         * - - - - - -
         * Usage and cycles:
         * STA Zero Page: 85(3)
         * STA Zero Page X: 95(4)
         * STA Absolute: 8D(4)
         * STA Absolute X: 9D(5)
         * STA Absolute Y: 99(5)
         * STA Indirect X: 81(6)
         * STA Indirect Y: 91(6)
         */
        StoreRegister( address, _a );
    }

    void STX( const u16 address ) // NOLINT
    {
        /*
         * @brief Store X Register in Memory
         * N Z C I D V
         * - - - - - -
         * Usage and cycles:
         * STX Zero Page: 86(3)
         * STX Zero Page Y: 96(4)
         * STX Absolute: 8E(4)
         */
        StoreRegister( address, _x );
    }

    void STY( const u16 address ) // NOLINT
    {
        /*
         * @brief Store Y Register in Memory
         * N Z C I D V
         * - - - - - -
         * Usage and cycles:
         * STY Zero Page: 84(3)
         * STY Zero Page X: 94(4)
         * STY Absolute: 8C(4)
         */
        StoreRegister( address, _y );
    }

    void ADC( u16 address )
    {
        /*
         * @brief Add Memory to Accumulator with Carry
         * N Z C I D V
         * + + + - - +
         * Usage and cycles:
         * ADC Immediate: 69(2)
         * ADC Zero Page: 65(3)
         * ADC Zero Page X: 75(4)
         * ADC Absolute: 6D(4)
         * ADC Absolute X: 7D(4+)
         * ADC Absolute Y: 79(4+)
         * ADC Indirect X: 61(6)
         * ADC Indirect Y: 71(5+)
         */
        u8 value = 0;

        if ( _instructionName == "*RRA" ) {
            value = Read( address ); // No cycle spend
        } else {
            value = ReadAndTick( address );
        }

        // Store the sum in a 16-bit variable to check for overflow
        u8 const  carry = IsFlagSet( Status::Carry ) ? 1 : 0;
        u16 const sum = _a + value + carry;

        // Set the carry flag if sum > 255
        // this means that there will be an overflow
        ( sum > 0xFF ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

        // If the lower part of sum is zero, set the zero flag
        ( ( sum & 0xFF ) == 0 ) ? SetFlags( Status::Zero ) : ClearFlags( Status::Zero );

        // Signed overflow is set if the sign bit is different in the accumulator and the result
        // e.g.
        // 1000 0001 + // << Accumulator: -127
        // 1000 0001   // << Value: -127
        // ---------
        // 0000 0010   // << Sum: 2. Sign bit is different, result is positive but should be
        // negative
        u8 const accumulatorSignBit = _a & 0b10000000;
        u8 const valueSignBit = value & 0b10000000;
        u8 const sumSignBit = sum & 0b10000000;
        ( accumulatorSignBit == valueSignBit && accumulatorSignBit != sumSignBit )
            ? SetFlags( Status::Overflow )
            : ClearFlags( Status::Overflow );

        // If bit 7 is set, set the negative flag
        ( sum & 0b10000000 ) != 0 ? SetFlags( Status::Negative ) : ClearFlags( Status::Negative );

        // Store the lower byte of the sum in the accumulator
        _a = sum & 0xFF;
    }

    void SBC( u16 address )
    {
        /* @brief Subtract Memory from Accumulator with Borrow
         * N Z C I D V
         * + + + - - +
         * Usage and cycles:
         * SBC Immediate: E9(2)
         * SBC Zero Page: E5(3)
         * SBC Zero Page X: F5(4)
         * SBC Absolute: ED(4)
         * SBC Absolute X: FD(4+)
         * SBC Absolute Y: F9(4+)
         * SBC Indirect X: E1(6)
         * SBC Indirect Y: F1(5+)
         *
         * --  Illegal  --
         * SBC Immediate: EB(2)
         */

        u8 value = 0;
        if ( _instructionName == "*ISC" ) {
            value = Read( address ); // 0 cycles
        } else {
            value = ReadAndTick( address );
        }
        // u8 const value = ReadAndTick( address );

        // Store diff in a 16-bit variable to check for overflow
        u8 const  carry = IsFlagSet( Status::Carry ) ? 0 : 1;
        u16 const diff = _a - value - carry;

        // Carry flag exists in the high byte?
        ( diff < 0x100 ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

        // If the lower part of diff is zero, set the zero flag
        ( ( diff & 0xFF ) == 0 ) ? SetFlags( Status::Zero ) : ClearFlags( Status::Zero );

        // Signed overflow is set if the sign bit is different in the accumulator and the result
        // e.g.
        // 0000 0001 - // << Accumulator: 1
        // 0000 0010   // << Value: 2
        // ---------
        // 1111 1111   // << Diff: 127. Sign bit is different
        u8 const accumulatorSignBit = _a & 0b10000000;
        u8 const valueSignBit = value & 0b10000000;
        u8 const diffSignBit = diff & 0b10000000;
        ( accumulatorSignBit != valueSignBit && accumulatorSignBit != diffSignBit )
            ? SetFlags( Status::Overflow )
            : ClearFlags( Status::Overflow );

        // If bit 7 is set, set the negative flag
        ( diff & 0b10000000 ) != 0 ? SetFlags( Status::Negative ) : ClearFlags( Status::Negative );

        // Store the lower byte of the diff in the accumulator
        _a = diff & 0xFF;
    }

    void INC( u16 address )
    {
        /*
         * @brief Increment Memory by One
         * N Z C I D V
         * + + - - - -
         * Usage and cycles:
         * INC Zero Page: E6(5)
         * INC Zero Page X: F6(6)
         * INC Absolute: EE(6)
         * INC Absolute X: FE(7)
         */
        u8 const value = ReadAndTick( address );
        Tick(); // Dummy write
        u8 const result = value + 1;
        SetZeroAndNegativeFlags( result );
        WriteAndTick( address, result );
    }

    void INX( u16 address )
    {
        /*
         * @brief Increment X Register by One
         * N Z C I D V
         * + + - - - -
         * Usage and cycles:
         * INX: E8(2)
         */
        (void) address;
        _x++;
        SetZeroAndNegativeFlags( _x );
    }

    void INY( u16 address )
    {
        /*
         * @brief Increment Y Register by One
         * N Z C I D V
         * + + - - - -
         * Usage and cycles:
         * INY: C8(2)
         */
        (void) address;
        _y++;
        SetZeroAndNegativeFlags( _y );
    }

    void DEC( u16 address )
    {
        /*
         * @brief Decrement Memory by One
         * N Z C I D V
         * + + - - - -
         * Usage and cycles:
         * DEC Zero Page: C6(5)
         * DEC Zero Page X: D6(6)
         * DEC Absolute: CE(6)
         * DEC Absolute X: DE(7)
         */
        u8 const value = ReadAndTick( address );
        Tick(); // Dummy write
        u8 const result = value - 1;
        SetZeroAndNegativeFlags( result );
        WriteAndTick( address, result );
    }

    void DEX( u16 address )
    {
        /*
         * @brief Decrement X Register by One
         * N Z C I D V
         * + + - - - -
         * Usage and cycles:
         * DEX: CA(2)
         */
        (void) address;
        _x--;
        SetZeroAndNegativeFlags( _x );
    }

    void DEY( u16 address )
    {
        /*
         * @brief Decrement Y Register by One
         * N Z C I D V
         * + + - - - -
         * Usage and cycles:
         * DEY: 88(2)
         */
        (void) address;
        _y--;
        SetZeroAndNegativeFlags( _y );
    }

    void CLC( const u16 address )
    {
        /* @brief Clear Carry Flag
         * N Z C I D V
         * - - 0 - - -
         *   Usage and cycles:
         *   CLC: 18(2)
         */
        (void) address;
        ClearFlags( Carry );
    }

    void CLI( const u16 address )
    {
        /* @brief Clear Interrupt Disable
         * N Z C I D V
         * - - - 0 - -
         *   Usage and cycles:
         *   CLI: 58(2)
         */
        (void) address;
        ClearFlags( InterruptDisable );
    }
    void CLD( const u16 address )
    {
        /* @brief Clear Decimal Mode
         * N Z C I D V
         * - - - - 0 -
         *   Usage and cycles:
         *   CLD: D8(2)
         */
        (void) address;
        ClearFlags( Decimal );
    }
    void CLV( const u16 address )
    {
        /* @brief Clear Overflow Flag
         * N Z C I D V
         * - - - - - 0
         *   Usage and cycles:
         *   CLV: B8(2)
         */
        (void) address;
        ClearFlags( Overflow );
    }

    void SEC( const u16 address )
    {
        /* @brief Set Carry Flag
         * N Z C I D V
         * - - 1 - - -
         *   Usage and cycles:
         *   SEC: 38(2)
         */
        (void) address;
        SetFlags( Carry );
    }

    void SED( const u16 address )
    {
        /* @brief Set Decimal Flag
         * N Z C I D V
         * - - - - 1 -
         *   Usage and cycles:
         *   SED: F8(2)
         */
        (void) address;
        SetFlags( Decimal );
    }

    void SEI( const u16 address )
    {
        /* @brief Set Interrupt Disable
         * N Z C I D V
         * - - - 1 - -
         *   Usage and cycles:
         *   SEI: 78(2)
         */
        (void) address;
        SetFlags( InterruptDisable );
    }

    void BPL( const u16 address )
    {
        /* @brief Branch if Positive
         * N Z C I D V
         * - - - - - -
         *   Usage and cycles:
         *   BPL: 10(2+)
         */
        BranchOnStatus( address, Status::Negative, false );
    }

    void BMI( const u16 address )
    {
        /* @brief Branch if Minus
         * N Z C I D V
         * - - - - - -
         *   Usage and cycles:
         *   BMI: 30(2+)
         */
        BranchOnStatus( address, Status::Negative, true );
    }

    void BVC( const u16 address )
    {
        /* @brief Branch if Overflow Clear
         * N Z C I D V
         * - - - - - -
         *   Usage and cycles:
         *   BVC: 50(2+)
         */
        BranchOnStatus( address, Status::Overflow, false );
    }

    void BVS( const u16 address )
    {
        /* @brief Branch if Overflow Set
         * N Z C I D V
         * - - - - - -
         *   Usage and cycles:
         *   BVS: 70(2+)
         */
        BranchOnStatus( address, Status::Overflow, true );
    }

    void BCC( const u16 address )
    {
        /* @brief Branch if Carry Clear
         * N Z C I D V
         * - - - - - -
         *   Usage and cycles:
         *   BCC: 90(2+)
         */
        BranchOnStatus( address, Status::Carry, false );
    }

    void BCS( const u16 address )
    {
        /* @brief Branch if Carry Set
         * N Z C I D V
         * - - - - - -
         *   Usage and cycles:
         *   BCS: B0(2+)
         */
        BranchOnStatus( address, Status::Carry, true );
    }

    void BNE( const u16 address )
    {
        /* @brief Branch if Not Equal
         * N Z C I D V
         * - - - - - -
         *   Usage and cycles:
         *   BNE: D0(2+)
         */
        BranchOnStatus( address, Status::Zero, false );
    }

    void BEQ( const u16 address )
    {
        /* @brief Branch if Equal
         * N Z C I D V
         * - - - - - -
         *   Usage and cycles:
         *   BEQ: F0(2+)
         */
        BranchOnStatus( address, Status::Zero, true );
    }

    void CMP( u16 address )
    {
        /* @brief Compare Memory and Accumulator
         * N Z C I D V
         * + + + - - -
         *   Usage and cycles:
         *   CMP Immediate: C9(2)
         *   CMP Zero Page: C5(3)
         *   CMP Zero Page X: D5(4)
         *   CMP Absolute: CD(4)
         *   CMP Absolute X: DD(4+)
         *   CMP Absolute Y: D9(4+)
         *   CMP Indirect X: C1(6)
         *   CMP Indirect Y: D1(5+)
         */
        CompareAddressWithRegister( address, _a );
    }

    void CPX( u16 address )
    {
        /* @brief Compare Memory and X Register
         * N Z C I D V
         * + + + - - -
         *   Usage and cycles:
         *   CPX Immediate: E0(2)
         *   CPX Zero Page: E4(3)
         *   CPX Absolute: EC(4)
         */
        CompareAddressWithRegister( address, _x );
    }

    void CPY( u16 address )
    {
        /* @brief Compare Memory and Y Register
         * N Z C I D V
         * + + + - - -
         *   Usage and cycles:
         *   CPY Immediate: C0(2)
         *   CPY Zero Page: C4(3)
         *   CPY Absolute: CC(4)
         */
        CompareAddressWithRegister( address, _y );
    }

    void PHA( const u16 address )
    {
        /* @brief Push Accumulator on Stack
         * N Z C I D V
         * - - - - - -
         *   Usage and cycles:
         *   PHA: 48(3)
         */
        (void) address;

        // Get the stack pointer
        const u8 stackPointer = GetStackPointer();

        // Push the accumulator onto the stack
        WriteAndTick( 0x0100 + stackPointer, GetAccumulator() );

        // Decrement the stack pointer
        SetStackPointer( stackPointer - 1 );
    }

    void PHP( const u16 address )
    {
        /* @brief Push Processor Status on Stack
         * N Z C I D V
         * - - - - - -
         *   Usage and cycles:
         *   PHP: 08(3)
         */
        (void) address;

        // Get the stack pointer
        const u8 stackPointer = GetStackPointer();

        // Set the Break flag before pushing the status register onto the stack
        u8 status = GetStatusRegister();
        status |= Break;

        // Push the modified status register onto the stack
        WriteAndTick( 0x0100 + stackPointer, status );

        SetStackPointer( stackPointer - 1 );
    }

    void PLA( const u16 address )
    {
        /* @brief Pop Accumulator from Stack
         * N Z C I D V
         * + + - - - -
         *   Usage and cycles:
         *   PLA: 68(4)
         */
        (void) address;

        // Increment the stack pointer first
        SetStackPointer( GetStackPointer() + 1 );

        // Get the accumulator from the stack and set the zero and negative flags
        SetAccumulator( ReadAndTick( 0x100 + GetStackPointer() ) );
        Tick(); // Dummy read
        SetZeroAndNegativeFlags( _a );
    }

    void PLP( const u16 address )
    {
        /* @brief Pop Processor Status from Stack
         * N Z C I D V
         * from stack
         *   Usage and cycles:
         *   PLP: 28(4)
         */
        (void) address;

        // Increment the stack pointer first
        SetStackPointer( GetStackPointer() + 1 );

        SetStatusRegister( ReadAndTick( 0x100 + GetStackPointer() ) );
        ClearFlags( Status::Break );
        Tick(); // Dummy read
        SetFlags( Status::Unused );
    }

    void TSX( const u16 address )
    {
        /* @brief Transfer Stack Pointer to X
         * N Z C I D V
         * + + - - - -
         *   Usage and cycles:
         *   TSX: BA(2)
         */
        (void) address;
        SetXRegister( GetStackPointer() );
        SetZeroAndNegativeFlags( GetXRegister() );
    }

    void TXS( const u16 address )
    {
        /* @brief Transfer X to Stack Pointer
         * N Z C I D V
         * - - - - - -
         *   Usage and cycles:
         *   TXS: 9A(2)
         */
        (void) address;
        SetStackPointer( GetXRegister() );
    }

    void ASL( u16 address )
    {
        /* @brief Arithmetic Shift Left
         * N Z C I D V
         * + + + - - -
         *   Usage and cycles:
         *   ASL Accumulator: 0A(2)
         *   ASL Zero Page: 06(5)
         *   ASL Zero Page X: 16(6)
         *   ASL Absolute: 0E(6)
         *   ASL Absolute X: 1E(7)
         */

        if ( _addrMode == "IMP" ) {
            u8 accumulator = GetAccumulator();
            // Set the carry flag if bit 7 is set
            ( accumulator & 0b10000000 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

            // Shift the accumulator left by one
            accumulator <<= 1;

            // Set the zero and negative flags
            SetZeroAndNegativeFlags( accumulator );

            // Set the new accumulator value
            SetAccumulator( accumulator );
        } else {
            u8 const value = ReadAndTick( address );

            Tick(); // simulate dummy write

            // Set the carry flag if bit 7 is set
            ( value & 0b10000000 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

            u8 const result = value << 1;

            // Set the zero and negative flags
            SetZeroAndNegativeFlags( result );

            // Write the result back to memory
            WriteAndTick( address, result );
        }
    }

    void LSR( u16 address )
    {
        /* @brief Logical Shift Right
         * N Z C I D V
         * + + + - - -
         *   Usage and cycles:
         *   LSR Accumulator: 4A(2)
         *   LSR Zero Page: 46(5)
         *   LSR Zero Page X: 56(6)
         *   LSR Absolute: 4E(6)
         *   LSR Absolute X: 5E(7)
         */

        if ( _addrMode == "IMP" ) {
            u8 accumulator = GetAccumulator();
            // Set the carry flag if bit 0 is set
            ( accumulator & 0b00000001 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

            // Shift the accumulator right by one
            accumulator >>= 1;

            // Set the zero and negative flags
            SetZeroAndNegativeFlags( accumulator );

            // Set the new accumulator value
            SetAccumulator( accumulator );
        } else {
            u8 const value = ReadAndTick( address );
            Tick(); // simulate dummy write

            // Set the carry flag if bit 0 is set
            ( value & 0b00000001 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

            u8 const result = value >> 1;

            // Set the zero and negative flags
            SetZeroAndNegativeFlags( result );

            // Write the result back to memory
            WriteAndTick( address, result );
        }
    }

    void ROL( u16 address )
    {
        /* @brief Rotate Left
         * N Z C I D V
         * + + + - - -
         *   Usage and cycles:
         *   ROL Accumulator: 2A(2)
         *   ROL Zero Page: 26(5)
         *   ROL Zero Page X: 36(6)
         *   ROL Absolute: 2E(6)
         *   ROL Absolute X: 3E(7)
         */

        const u8 carry = IsFlagSet( Status::Carry ) ? 1 : 0;
        if ( _addrMode == "IMP" ) {
            u8 accumulator = GetAccumulator();

            // Set the carry flag if bit 7 is set
            ( accumulator & 0b10000000 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

            // Shift the accumulator left by one
            accumulator <<= 1;

            // Add the carry to the accumulator
            accumulator |= carry;

            // Set the zero and negative flags
            SetZeroAndNegativeFlags( accumulator );

            // Set the new accumulator value
            SetAccumulator( accumulator );
        } else {
            u8 const value = ReadAndTick( address );
            Tick(); // dummy write

            // Set the carry flag if bit 7 is set
            ( value & 0b10000000 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

            u8 result = value << 1;
            result |= carry;

            // Set the zero and negative flags
            SetZeroAndNegativeFlags( result );

            // Write the result back to memory
            WriteAndTick( address, result );
        }
    }

    void ROR( u16 address )
    {
        /* @brief Rotate Right
         * N Z C I D V
         * + + + - - -
         *   Usage and cycles:
         *   ROR Accumulator: 6A(2)
         *   ROR Zero Page: 66(5)
         *   ROR Zero Page X: 76(6)
         *   ROR Absolute: 6E(6)
         *   ROR Absolute X: 7E(7)
         */

        const u8 carry = IsFlagSet( Status::Carry ) ? 1 : 0;

        if ( _addrMode == "IMP" ) { // implied mode
            u8 accumulator = GetAccumulator();

            // Set the carry flag if bit 0 is set
            ( accumulator & 0b00000001 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

            // Shift the accumulator right by one
            accumulator >>= 1;

            // Add the carry to the accumulator
            accumulator |= carry << 7;

            // Set the zero and negative flags
            SetZeroAndNegativeFlags( accumulator );

            // Set the new accumulator value
            SetAccumulator( accumulator );
        } else { // Memory mode
            u8 const value = ReadAndTick( address );
            Tick(); // simulate dummy write

            // Set the carry flag if bit 0 is set
            ( value & 0b00000001 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

            u8 result = value >> 1;
            result |= carry << 7;

            // Set the zero and negative flags
            SetZeroAndNegativeFlags( result );

            // Write the result back to memory
            WriteAndTick( address, result );
        }
    }

    void JMP( u16 address )
    {
        /* @brief Jump to New Location
         * N Z C I D V
         * - - - - - -
         *   Usage and cycles:
         *   JMP Absolute: 4C(3)
         *   JMP Indirect: 6C(5)
         */
        _pc = address;
    }

    void JSR( u16 address )
    {
        /* @brief Jump to Sub Routine, Saving Return Address
         * N Z C I D V
         * - - - - - -
         *   Usage and cycles:
         *   JSR Absolute: 20(6)
         */
        u16 const returnAddress = _pc - 1;
        Tick(); // Additional read here, probably for timing purposes
        StackPush( ( returnAddress >> 8 ) & 0xFF );
        StackPush( returnAddress & 0xFF );
        _pc = address;
    }

    void RTS( const u16 address )
    {
        /* @brief Return from Subroutine
         * N Z C I D V
         * - - - - - -
         *   Usage and cycles:
         *   RTS: 60(6)
         */
        (void) address;
        u16 const low = StackPop();
        u16 const high = StackPop();
        _pc = ( high << 8 ) | low;
        Tick(); // Account for reading the new address
        _pc++;
        Tick(); // Account for reading the next pc value
    }

    void RTI( const u16 address )
    {
        /* @brief Return from Interrupt
         * N Z C I D V
         * from stack
         *   Usage and cycles:
         *   RTI: 40(6)
         */
        (void) address;
        u8 const status = StackPop();

        // Ignore the break flag and ensure the unused flag (bit 5) is set
        _p = ( status & ~Break ) | Unused;

        u16 const low = StackPop();
        u16 const high = StackPop();
        _pc = ( high << 8 ) | low;
        Tick(); // Account for reading the new address
    }

    void BRK( const u16 address )
    {
        /* @brief Force Interrupt
         * N Z C I D V
         * from stack
         *   Usage and cycles:
         *   BRK: 00(7)
         * Cycles:
         *   Read opcode: 1, Read padding byte: 1
         *   Push PC(2): 2, Push status(1): 1
         *   Read vector low: 1, Read vector high: 1
         */
        (void) address;
        _pc++; // padding byte

        // Push pc to the stack
        StackPush( _pc >> 8 );     // 1 cycle
        StackPush( _pc & 0x00FF ); // 1 cycle

        // Push status with break and unused flag set (ignored when popped)
        StackPush( _p | Break | Unused );

        // Set PC to the value at the interrupt vector (0xFFFE)
        u16 const low = ReadAndTick( 0xFFFE );
        u16 const high = ReadAndTick( 0xFFFF );
        _pc = ( high << 8 ) | low;

        // Set the interrupt disable flag
        SetFlags( InterruptDisable );
    }

    void AND( u16 address )
    {
        /* @brief XOR Memory with Accumulator
         * N Z C I D V
         * + + - - - -
         *   Usage and cycles:
         *   AND Immediate: 29(2)
         *   AND Zero Page: 25(3)
         *   AND Zero Page X: 35(4)
         *   AND Absolute: 2D(4)
         *   AND Absolute X: 3D(4+)
         *   AND Absolute Y: 39(4+)
         *   AND Indirect X: 21(6)
         *   AND Indirect Y: 31(5+)
         */
        u8 const value = ReadAndTick( address );
        _a &= value;
        SetZeroAndNegativeFlags( _a );
    }

    void ORA( const u16 address )
    {
        /* @brief OR Memory with Accumulator
         * N Z C I D V
         * + + - - - -
         *   Usage and cycles:
         *   ORA Immediate: 09(2)
         *   ORA Zero Page: 05(3)
         *   ORA Zero Page X: 15(4)
         *   ORA Absolute: 0D(4)
         *   ORA Absolute X: 1D(4+)
         *   ORA Absolute Y: 19(4+)
         *   ORA Indirect X: 01(6)
         *   ORA Indirect Y: 11(5+)
         */

        u8 const value = ReadAndTick( address ); // 1 cycle
        _a |= value;
        SetZeroAndNegativeFlags( _a );
    }

    void EOR( const u16 address )
    {
        /* @brief XOR Memory with Accumulator
         * N Z C I D V
         * + + - - - -
         *   Usage and cycles:
         *   EOR Immediate: 49(2)
         *   EOR Zero Page: 45(3)
         *   EOR Zero Page X: 55(4)
         *   EOR Absolute: 4D(4)
         *   EOR Absolute X: 5D(4+)
         *   EOR Absolute Y: 59(4+)
         *   EOR Indirect X: 41(6)
         *   EOR Indirect Y: 51(5+)
         */
        u8 const value = ReadAndTick( address );
        _a ^= value;
        SetZeroAndNegativeFlags( _a );
    }

    void BIT( const u16 address )
    {
        /* @brief Test Bits in Memory with Accumulator
         * Performs AND between accumulator and memory, but does not store the result
         * N Z C I D V
         * + + - - - +
         *   Usage and cycles:
         *   BIT Zero Page: 24(3)
         *   BIT Absolute: 2C(4)
         */

        u8 const value = ReadAndTick( address );
        SetZeroAndNegativeFlags( _a & value );

        // Set overflow flag to bit 6 of value
        ( value & 0b01000000 ) != 0 ? SetFlags( Status::Overflow ) : ClearFlags( Status::Overflow );

        // Set negative flag to bit 7 of value
        ( value & 0b10000000 ) != 0 ? SetFlags( Status::Negative ) : ClearFlags( Status::Negative );
    }

    void TAX( const u16 address )
    {
        /* @brief Transfer Accumulator to X Register
         * N Z C I D V
         * + + - - - -
         *   Usage and cycles:
         *   TAX: AA(2)
         */
        (void) address;
        SetXRegister( GetAccumulator() );
        SetZeroAndNegativeFlags( GetXRegister() );
    }

    void TXA( const u16 address )
    {
        /* @brief Transfer X Register to Accumulator
         * N Z C I D V
         * + + - - - -
         *   Usage and cycles:
         *   TXA: 8A(2)
         */
        (void) address;
        SetAccumulator( GetXRegister() );
        SetZeroAndNegativeFlags( GetAccumulator() );
    }

    void TAY( const u16 address )
    {
        /* @brief Transfer Accumulator to Y Register
         * N Z C I D V
         * + + - - - -
         *   Usage and cycles:
         *   TAY: A8(2)
         */
        (void) address;
        SetYRegister( GetAccumulator() );
        SetZeroAndNegativeFlags( GetYRegister() );
    }

    void TYA( const u16 address )
    {
        /* @brief Transfer Y Register to Accumulator
         * N Z C I D V
         * + + - - - -
         *   Usage and cycles:
         *   TYA: 98(2)
         */
        (void) address;
        SetAccumulator( GetYRegister() );
        SetZeroAndNegativeFlags( GetAccumulator() );
    }

    /*
    ################################################################
    ||                                                            ||
    ||                      Illegal Opcodes                       ||
    ||                                                            ||
    ################################################################
    */

    void NOP2( u16 address ) // NOLINT
    {
        /*
         * @brief No operation, has an additional cycle
         * N Z C I D V
         * - - - - - -
         * --  Illegal  --
         * NOP Immediate: 80(2)
         * NOP Immediate: 82(2)
         * NOP Immediate: 89(2)
         * NOP Immediate: C2(2)
         * NOP Immediate: E2(2)
         * NOP Zero Page: 04(3)
         * NOP Zero Page: 44(3)
         * NOP Zero Page: 64(3)
         * NOP Zero Page X: 14(4)
         * NOP Zero Page X: 34(4)
         * NOP Zero Page X: 54(4)
         * NOP Zero Page X: 74(4)
         * NOP Zero Page X: D4(4)
         * NOP Zero Page X: F4(4)
         * NOP Absolute: 0C(4)
         * NOP Absolute: 1C(4)
         * NOP Absolute: 3C(4)
         * NOP Absolute: 5C(4)
         * NOP Absolute: 7C(4)
         * NOP Absolute: DC(4)
         * NOP Absolute: FC(4)
         */
        Tick();
        (void) address;
    }
    void JAM( const u16 address ) // NOLINT
    {
        /* @brief Illegal Opcode
         * Freezes the hardware, usually never called
         * Tom Harte tests include these, though, so for completeness, we'll add them
         */
        (void) address;
        for ( int i = 0; i < 9; i++ ) {
            Tick();
        }
    }

    void SLO( const u16 address )
    {
        /* @brief Illegal opcode: combines ASL and ORA
         * N Z C I D V
         * + + + - - -
         *   Usage and cycles:
         *   SLO Zero Page: 07(5)
         *   SLO Zero Page X: 17(6)
         *   SLO Absolute: 0F(6)
         *   SLO Absolute X: 1F(7)
         *   SLO Absolute Y: 1B(7)
         *   SLO Indirect X: 03(8)
         *   SLO Indirect Y: 13(8)
         */
        ASL( address );

        // ORA is side effect, no cycles are spent
        u8 const value = Read( address ); // 0 cycle
        _a |= value;
        SetZeroAndNegativeFlags( _a );
    }

    void SAX( const u16 address ) // NOLINT
    {
        /* @brief Illegal opcode: combines STX and AND
         * N Z C I D V
         * - - - - - -
         *   Usage and cycles:
         *   SAX Zero Page: 87(3)
         *   SAX Zero Page Y: 97(4)
         *   SAX Indirect X: 83(6)
         *   SAX Absolute: 8F(4)
         */
        WriteAndTick( address, _a & _x );
    }

    void LXA( const u16 address )
    {
        /* @brief Illegal opcode: combines LDA and LDX
         * N Z C I D V
         * + + - - - -
         *   Usage and cycles:
         *   LXA Immediate: AB(2)
         */

        u8 const magicConstant = 0xEE;
        u8 const value = ReadAndTick( address );

        u8 const result = ( ( _a | magicConstant ) & value );
        _a = result;
        _x = result;
        SetZeroAndNegativeFlags( _a );
    }

    void LAX( const u16 address )
    {
        /* @brief Illegal opcode: combines LDA and LDX
         * N Z C I D V
         * + + - - - -
         *   Usage and cycles:
         *   LAX Zero Page: A7(3)
         *   LAX Zero Page Y: B7(4)
         *   LAX Absolute: AF(4)
         *   LAX Absolute Y: BF(4+)
         *   LAX Indirect X: A3(6)
         *   LAX Indirect Y: B3(5+)
         */
        u8 const value = ReadAndTick( address );
        SetAccumulator( value );
        SetXRegister( value );
        SetZeroAndNegativeFlags( value );
    }

    void ARR( const u16 address )
    {
        /* @brief Illegal opcode: combines AND and ROR
         * N Z C I D V
         * + + + - - +
         *   Usage and cycles:
         *   ARR Immediate: 6B(2)
         */

        // A & operand
        u8 value = _a & ReadAndTick( address );

        // ROR
        u8 const carryIn = IsFlagSet( Status::Carry ) ? 0x80 : 0x00;
        value = ( value >> 1 ) | carryIn;

        _a = value;

        // Set flags
        SetZeroAndNegativeFlags( _a );

        // Adjust C and V flags according to the ARR rules
        // C = bit 6 of A
        ( _a & 0x40 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

        // V = bit 5 XOR bit 6
        bool const isOverflow = ( ( _a & 0x40 ) != 0 ) ^ ( ( _a & 0x20 ) != 0 );
        ( isOverflow ) ? SetFlags( Status::Overflow ) : ClearFlags( Status::Overflow );
    }

    void ALR( const u16 address )
    {
        /* @brief Illegal opcode: combines AND and LSR
         * N Z C I D V
         * + + + - - -
         *   Usage and cycles:
         *   ALR Immediate: 4B(2)
         */
        AND( address );

        u8 const value = GetAccumulator();
        ( value & 0b00000001 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );
        u8 const result = value >> 1;
        SetZeroAndNegativeFlags( result );
        _a = result;
    }

    void RRA( const u16 address )
    {
        /* @brief Illegal opcode: combines ROR and ADC
         * N Z C I D V
         * + + + - - -
         *   Usage and cycles:
         *   RRA Zero Page: 67(5)
         *   RRA Zero Page X: 77(6)
         *   RRA Absolute: 6F(6)
         *   RRA Absolute X: 7F(7)
         *   RRA Absolute Y: 7B(7)
         *   RRA Indirect X: 63(8)
         *   RRA Indirect Y: 73(8)
         */
        ROR( address );
        ADC( address );
    }

    void SRE( const u16 address )
    {
        /* @brief Illegal opcode: combines LSR and EOR
         * N Z C I D V
         * + + + - - -
         *   Usage and cycles:
         *   SRE Zero Page: 47(5)
         *   SRE Zero Page X: 57(6)
         *   SRE Absolute: 4F(6)
         *   SRE Absolute X: 5F(7)
         *   SRE Absolute Y: 5B(7)
         *   SRE Indirect X: 43(8)
         *   SRE Indirect Y: 53(8)
         */
        LSR( address );

        // Free side effect
        u8 const value = Read( address ); // 0 cycle
        _a ^= value;
        SetZeroAndNegativeFlags( _a );
    }

    void RLA( const u16 address )
    {
        /* @brief Illegal opcode: combines ROL and AND
         * N Z C I D V
         * + + + - - -
         *   Usage and cycles:
         *   RLA Zero Page: 27(5)
         *   RLA Zero Page X: 37(6)
         *   RLA Absolute: 2F(6)
         *   RLA Absolute X: 3F(7)
         *   RLA Absolute Y: 3B(7)
         *   RLA Indirect X: 23(8)
         *   RLA Indirect Y: 33(8)
         */
        ROL( address );

        // Free side effect
        u8 const value = Read( address ); // 0 cycle
        _a &= value;
        SetZeroAndNegativeFlags( _a );
    }

    void DCP( const u16 address )
    {
        /* @brief Illegal opcode: combines DEC and CMP
         * N Z C I D V
         * + + + - - -
         *   Usage and cycles:
         *   DCP Zero Page: C7(5)
         *   DCP Zero Page X: D7(6)
         *   DCP Absolute: CF(6)
         *   DCP Absolute X: DF(7)
         *   DCP Absolute Y: DB(7)
         *   DCP Indirect X: C3(8)
         *   DCP Indirect Y: D3(8)
         */
        DEC( address );
        CMP( address );
    }

    void ISC( const u16 address )
    {
        /* @brief Illegal opcode: combines INC and SBC
         * N Z C I D V
         * + + + - - +
         *   Usage and cycles:
         *   ISC Zero Page: E7(5)
         *   ISC Zero Page X: F7(6)
         *   ISC Absolute: EF(6)
         *   ISC Absolute X: FF(7)
         *   ISC Absolute Y: FB(7)
         *   ISC Indirect X: E3(8)
         *   ISC Indirect Y: F3(8)
         */
        INC( address );
        SBC( address );
    }

    void ANC( const u16 address )
    {
        /* @brief Illegal opcode: combines AND and Carry
         * N Z C I D V
         * + + + - - -
         *   Usage and cycles:
         *   ANC Immediate: 0B(2)
         *   ANC Immediate: 2B(2)
         */
        AND( address );
        ( IsFlagSet( Status::Negative ) ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );
    }

    void SBX( const u16 address )
    {
        /* @brief Illegal opcode: SBX (a.k.a. AXS) combines CMP and DEX
         *        (A & X) - immediate -> X
         * Sets flags like CMP:
         *   N Z C I D V
         *   + + + - - -
         *
         * Usage and cycles:
         *   SBX Immediate: CB (2 bytes, 2 cycles)
         */
        u8 const  operand = ReadAndTick( address );
        u8 const  left = ( _a & _x );
        u16 const diff = static_cast<uint16_t>( left ) - static_cast<uint16_t>( operand );
        _x = static_cast<u8>( diff & 0xFF );
        ( ( diff & 0x100 ) == 0 ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );
        SetZeroAndNegativeFlags( _x );
    }

    void LAS( const u16 address )
    {
        /* @brief Illegal opcode: LAS(LAR), combines LDA/TSX
         * M AND SP -> A, X, SP
         *
         * N Z C I D V
         * + + - - - -
         *   Usage and cycles:
         *   LAS Absolute Y: BB(4+)
         */
        u8 const memVal = ReadAndTick( address );
        u8 const sp = GetStackPointer();
        u8 const result = memVal & sp;

        _a = result;
        _x = result;
        _s = result;

        SetZeroAndNegativeFlags( result );
    }

    void ANE( const u16 address )
    {
        /* @details Illegal opcode: ANE, combines AND and EOR
          * OR X + AND oper

          A base value in A is determined based on the contets of A and a constant, which may be typically
          $00, $ff, $ee, etc. The value of this constant depends on temerature, the chip series, and maybe
          other factors, as well. In order to eliminate these uncertaincies from the equation, use either 0 as
          the operand or a value of $FF in the accumulator.

          (A OR CONST) AND X AND oper -> A
          N	Z	C	I	D	V
          +	+	-	-	-	-
          addressing	assembler	opc	bytes	cycles
          immediate	ANE #oper	8B	2	2  	††

          Usage and cycles:
          * ANE Immediate: 8B(2)
        */
        u8 const operand = ReadAndTick( address );
        u8 const constant = 0xEE;

        // Compute: (A OR constant) AND X AND operand.
        u8 const result = ( _a | constant ) & _x & operand;
        _a = result;

        SetZeroAndNegativeFlags( _a );
    }
};
