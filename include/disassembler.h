#pragma once
#include <cstdint>
#include <string>
#include "cpu.h"
#include "utils.h"

using u8 = uint8_t;
using u16 = uint16_t;
using str = std::string;

using namespace std;

namespace disassembler
{
struct Tokens {
    u8  opcode{};
    str name;
    str prefix;   // Punctuation before operand (e.g., "#" or "(")
    str operand;  // The actual hex literal, e.g., "$00" or "$0000"
    str postfix;  // Punctuation immediately after operand (e.g., "," or ")" or "),")
    str reg;      // Register token ("X" or "Y") if applicable
    str postfix2; // Any trailing punctuation (e.g., a closing parenthesis)
};

// Prefix constants
const str immPrefix = "#";
const str base16Prefix = "$";

static Tokens parse( u8 opcode, u8 operand )
{
    str const &mode = gAddressingModes[opcode];
    str const &name = gInstructionNames[opcode];

    Tokens inst;
    inst.opcode = opcode;
    inst.name = name;
    inst.prefix = "";
    inst.postfix = "";
    inst.postfix2 = "";
    inst.reg = "";
    inst.operand = base16Prefix + utils::toHex( operand, 2 );

    if ( mode == "IMP" ) {
        inst.operand = "";
    } else if ( mode == "IMM" ) {
        // Immediate e.g. LDA #$00
        inst.prefix = immPrefix;
    } else if ( mode == "ZPGX" ) {
        // Zero Page,X: e.g. LDA $00,X
        inst.postfix = ","; // punctuation before register
        inst.reg = "X";
    } else if ( mode == "ZPGY" ) {
        // Zero Page,Y: e.g. LDX $00,Y
        inst.postfix = ",";
        inst.reg = "Y";
    } else if ( mode == "ABSX" ) {
        // Absolute,X: e.g. LDA $0000,X
        inst.operand = base16Prefix + utils::toHex( operand, 4 );
        inst.postfix = ",";
        inst.reg = "X";
    } else if ( mode == "ABSY" ) {
        // Absolute,Y: e.g. LDA $0000,Y
        inst.operand = base16Prefix + utils::toHex( operand, 4 );
        inst.postfix = ",";
        inst.reg = "Y";
    } else if ( mode == "IND" ) {
        // Indirect: e.g. JMP ($0000)
        inst.prefix = "(";
        inst.operand = base16Prefix + utils::toHex( operand, 4 );
        inst.postfix = ")";
    } else if ( mode == "INDX" ) {
        // Indexed Indirect: e.g. LDA ($00,X)
        inst.prefix = "(";
        inst.postfix = ",";
        inst.reg = "X";
        inst.postfix2 = ")";
    } else if ( mode == "INDY" ) {
        // Indirect Indexed: e.g. LDA ($00),Y
        inst.prefix = "(";
        inst.postfix = "),";
        inst.reg = "Y";
    } else if ( mode == "REL" || mode == "ZPG" ) {
        inst.operand = base16Prefix + utils::toHex( operand, 4 );
    }

    return inst;
}

// function to read rom file and grab a vector full of tokens, used for debugger
inline vector<Tokens> disassemble( const str &romPath )
{
    vector<Tokens> tokens;
    std::ifstream  file( romPath, std::ios::binary );
    if ( !file.is_open() ) {
        throw std::runtime_error( "Failed to open ROM file: " + romPath );
    }
    // skip the header (16 bytes)
    file.seekg( 16 );

    // read the rest of the file

    return tokens;
}

} // namespace disassembler
