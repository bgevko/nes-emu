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

- [x] `0x00` BRK
- [x] `0x21` AND X-indexed indirect
- [x] `0x81` STA X-indexed indirect
- [x] `0x84` STY Zero page
- [x] `0x85` STA Zero page
- [x] `0x86` STX Zero page
- [x] `0x8C` STY Absolute
- [x] `0x8D` STA Absolute
- [x] `0x8E` STX Absolute
- [x] `0x91` STA Indirect Y-indexed
- [x] `0x94` STY Zero page X-indexed
- [x] `0x95` STA Zero page X-indexed
- [x] `0x96` STX Zero page Y-indexed
- [x] `0x99` STA Absolute Y-indexed
- [x] `0x9D` STA Absolute X-indexed
- [x] `0xA0` LDY Immediate
- [x] `0xA1` LDA X-indexed indirect
- [x] `0xA2` LDX Immediate
- [x] `0xA4` LDY Zero page
- [x] `0xA5` LDA Zero page
- [x] `0xA6` LDX Zero page
- [x] `0xA9` LDA Immediate
- [x] `0xAC` LDY Absolute
- [x] `0xAD` LDA Absolute
- [x] `0xAE` LDX Absolute
- [x] `0xB1` LDA Indirect Y-indexed
- [x] `0xB4` LDY Zero page X-indexed
- [x] `0xB5` LDA Zero page X-indexed
- [x] `0xB6` LDX Zero page Y-indexed
- [x] `0xB9` LDA Absolute Y-indexed
- [x] `0xBC` LDY Absolute X-indexed
- [x] `0xBD` LDA Absolute X-indexed
- [x] `0xBE` LDX Absolute Y-indexed

See the full task list [here](tasks.md).
