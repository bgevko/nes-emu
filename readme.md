# NES Emulator in C++

I'm building a A Nintendo Entertainment System (NES) emulator in C++ for fun!
My purpose is to just have fun and learn more about how the NES, and computers in general, work.

## Table of Contents

1. [Project Overview](#Project-Overview)
2. [Features](#Features)
3. [Current Progress](#Current-Progress)

## Project Overview

The NES consists of three major components: the CPU, the PPU, and the APU. The CPU is a Ricoh 2A03, which is a modified 6502 processor. The PPU generates video output and the APU generates audio output. The NES has 2KB of RAM, 2KB of video RAM, and 32KB of ROM. The NES has a 16-bit address bus and an 8-bit data bus. To complete this project, I will need to implement the CPU, PPU, and APU, as well as the memory mapping and cartridge loading.

## Features

- [ ] CPU <- We are here
- [ ] PPU
- [ ] APU

## Current Progress

I'm currently working on the CPU. I have implemented the following features:

### Addressing Modes

- [x] Immediate
- [x] Zero Page
- [x] Zero Page X
- [x] Zero Page Y
- [x] Relative
- [x] Absolute
- [x] Absolute X
- [x] Absolute Y
- [x] Indirect
- [x] Indirect X
- [x] Indirect Y

### Implemented Instructions

- [x] `0x00` - BRK: Force Interrupt
- [x] `0x21` - AND: Logical AND (X-indexed indirect)
- [x] `0xA0` - LDY: Load Y Register (Immediate)
- [x] `0xA1` - LDA: Load Accumulator (X-indexed indirect)
- [x] `0xA2` - LDX: Load X Register (Immediate)
- [x] `0xA4` - LDY: Load Y Register (Zero Page)
- [x] `0xA5` - LDA: Load Accumulator (Zero Page)
- [x] `0xA6` - LDX: Load X Register (Zero Page)
- [x] `0xA9` - LDA: Load Accumulator (Immediate)
- [x] `0xAC` - LDY: Load Y Register (Absolute)
- [x] `0xAD` - LDA: Load Accumulator (Absolute)
- [x] `0xAE` - LDX: Load X Register (Absolute)
- [x] `0x8D` - STA: Store Accumulator (Absolute)

See the full task list [here](tasks.md).
