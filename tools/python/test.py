import unittest
import emu

class TestExample(unittest.TestCase):
    

    def test_bug_1(self):
        # there's a bug where the emulator jumps to a different address, even though
        # it's supposed to continue to jump to the same address indefinitely.

        e = emu.Emulator()
        e.load("../../roms/palette.nes")
        e.debug_reset()
        pal0 = e.ppu_read(0x3F00)
        steps = 0
        while pal0 == e.ppu_read(0x3F00):
            e.step()
            steps += 1
        print(f"Steps: {steps}")
        e.log()

        
    def test_another_example(self):
        """Another example test case."""
        self.assertTrue(isinstance("hello", str))

if __name__ == "__main__":
    unittest.main()
