#!/bin/bash

# This assembles the demo 6502 assembly file in this directory
# To get this to work, you need the cl65 assembler, which is part of the cc65 suite.
# You can get it here: https://cc65.github.io/

# Input: demo.s
# Params: --verbose --target nes -o demo.nes

cl65 --verbose --target nes -o ../../tests/roms/custom.nes custom.s
