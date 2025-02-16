# CLI for Mac OS
This is a guide for setting up a CLI development environment on Mac OS

From the root of the project, run the setup script
```bash
.scripts/env-check.sh
```

![Screenshot 2025-02-13 at 3 14 54 PM](https://github.com/user-attachments/assets/fb2aded7-361b-412f-87b2-b40445183b7a)

Install any missing tools
```bash
brew install llvm cmake pkg-config ninja
```

After installation, all tools should be available except for `vcpkg`

![Screenshot 2025-02-13 at 3 18 26 PM](https://github.com/user-attachments/assets/674a9d6e-590f-4f86-9a9c-1dc8eb3a35e1)

Install Vcpkg. Be mindful of file permissions. I'll place mine in user root `~/`, which requires no additional permission settings on my machine
```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg && ./bootstrap-vcpkg.sh -disableMetrics
```

Add the `VCPKG_ROOT` to your environment variables and your PATH variable. I recommend adding it to your shell profile for persistence.
```bash
export VCPKG_ROOT=~/vcpkg
export PATH=$VCPKG_ROOT:$PATH
```

Reload your shell and run `./scripts/env-check.sh` again.

![Screenshot 2025-02-13 at 3 39 53 PM](https://github.com/user-attachments/assets/db2412bd-f396-4f23-b5d0-79256e2c6865)

From the project root, install the vcpkg dependencies (sourced from `vcpkg.json`)
```bash
vcpkg install
```

You'll want to run this command each time you add a new package to `vcpkg.json`. If you're adding a new package, don't forget to paste the build targets from the installation output in to `CMakeLists.txt`.

Build the project
```bash
scripts/build.sh
```

Run the executable and ensure all the tests pass
```bash
build/emu
```
```bash
scripts/test.sh
```

See the [CLI Tools](https://github.com/bgevko/nes-emu/blob/main/docs/CLI_Tools.md) document for more CLI options

---
