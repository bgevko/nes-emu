## About

This directory turns the emulator into a Python module, allowing for easier testing and debugging.

If you're not on a silicon Mac, you'll need to build the module to use it. You'll need the `pybind11` library installed globally, and then you can run
`scripts/build.sh` from this directory, or `scripts/build.sh python` from the project root.

After it builds, you can use it as a regular Python module:

```python
import emu
```

`emu.py` for some usage examples.
