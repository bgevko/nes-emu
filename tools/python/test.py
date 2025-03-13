import unittest
import emu

class TestExample(unittest.TestCase):
    
    def setUp(self):
        e = emu.Emulator() # ignore the lint warning, this does exist
        e.preset() # loads testing/roms/custom.nes

    def bug_1(self):
        # there's a bug where the emulator jumps to a different address, even though
        # it's supposed to continue to jump to the same address indefinitely.
        self.assertEqual(1 + 1, 2)
        pass
        
    def test_another_example(self):
        """Another example test case."""
        self.assertTrue(isinstance("hello", str))

if __name__ == "__main__":
    unittest.main()
