# NES Emulator in C++

I'm building a A Nintendo Entertainment System (NES) emulator in C++ to learn more about the NES, computer architecture, and emulation.

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

See the full task list [here](tasks.md).
