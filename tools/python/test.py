import unittest
import emu

class TestExample(unittest.TestCase):
    

    def test_bug_1(self):
        # there's a bug where the emulator jumps to a different address, even though
        # it's supposed to continue to jump to the same address indefinitely.

        e = emu.Emulator()
        e.load("test_roms/rom1.nes")
        e.debug_reset()

        # Jump to start of infinite loop
        steps = 15960
        e.step(steps)
        self.assertEqual(e.pc, 0x8059)
        e.step()
        e.step()

        # Continue until 100k steps. pc step one, and remain at 0x805E
        while steps <= 100000:
            e.step()
            if e.pc != 0x805E:
                e.log()
                print(" Failure at step ", steps)
                self.fail()
            steps += 1

        
    def test_another_example(self):
        """Another example test case."""
        self.assertTrue(isinstance("hello", str))

if __name__ == "__main__":
    unittest.main()
