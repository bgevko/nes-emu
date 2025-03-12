## About

This directory provides a way to build the emulator as a Python module.

This allows to run exposed methods in Python's interactive shell. Very useful for debugging and creating logs

## How to use

This module is optional and does not interfere with the main build in any way.
It needs the `pybind11` library in order to build.

To build, run from the root directory:

```bash
scripts/build.sh cli
```

or just `scripts/build` from this directory.

Once built, run the Python shell `Python3` and import the module:

```python
import emu_module
```

Then, you can run exposed methods listed in `emu_module.cpp`.

This is fine for quick testing, but the recommended method is to bootstrap from `cli.py`. It makes many useful functions bindings globally available.

```bash
Python3 -i cli.py
```
