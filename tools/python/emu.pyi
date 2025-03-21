# emu.pyi
# This is a Python stub file for the Emulator class. It makes the python linter happy, but these do need to be updated
# and maintained.
from typing import Any

class Emulator:
    def __init__(self) -> None: ...
    # CPU Getters
    @property
    def cpu_cycles(self) -> int: ...
    @property
    def a(self) -> int: ...
    @property
    def x(self) -> int: ...
    @property
    def y(self) -> int: ...
    @property
    def sp(self) -> int: ...
    @property
    def pc(self) -> int: ...
    @property
    def p(self) -> int: ...
    @property
    def carry_flag(self) -> int: ...
    @property
    def zero_flag(self) -> int: ...
    @property
    def interrupt_flag(self) -> int: ...
    @property
    def decimal_flag(self) -> int: ...
    @property
    def break_flag(self) -> int: ...
    @property
    def overflow_flag(self) -> int: ...
    @property
    def negative_flag(self) -> int: ...

    # CPU Setters
    def set_cycles(self, value: int) -> None: ...
    def set_a(self, value: int) -> None: ...
    def set_x(self, value: int) -> None: ...
    def set_y(self, value: int) -> None: ...
    def set_sp(self, value: int) -> None: ...
    def set_pc(self, value: int) -> None: ...
    def set_p(self, value: int) -> None: ...

    # PPU Getters
    @property
    def nmi(self) -> int: ...
    @property
    def vblank(self) -> int: ...
    @property
    def scanline(self) -> int: ...
    @property
    def ppu_cycles(self) -> int: ...
    @property
    def frame(self) -> int: ...

    # PPU Setters
    def set_scanline(self, value: int) -> None: ...
    def set_ppu_cycles(self, value: int) -> None: ...

    # Cartrdige Getters
    def did_mapper_load(self) -> bool: ...
    @property
    def does_mapper_exist(self) -> bool: ...
    
    # Cartridge Setters
    
    # Methods
    def load(self, path: str) -> None: ...
    def preset(self) -> None: ...
    def debug_reset(self) -> None: ...
    def log(self) -> None: ...
    def step(self, n: int = 1) -> None: ...
    def enable_mesen_trace(self, n: int = 100) -> None: ...
    def disable_mesen_trace(self) -> None: ...
    def print_mesen_trace(self) -> None: ...
    def read(self, addr: int) -> int: ...
    def ppu_read(self, addr: int) -> int: ...
    
    @staticmethod
    def test() -> None: ...
