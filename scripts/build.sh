#!/bin/bash
set -euo pipefail

# scripts/build.sh
# Usage: ./scripts/build.sh [full|tests]
#  - "full" (or no argument): build the full emulator (SDL frontend, core, and tests)
#  - "tests": build only the test executables (core + tests)
#
# This script automatically selects a build directory based on whether it's running inside Docker.
export BUILD_DIR="build"

# Determine build configuration based on the command-line argument.
# When running in Docker, we default to the "tests" preset.
if [ -z "${1:-}" ] || [ "$1" == "full" ]; then
  if [ -f /.dockerenv ]; then
    echo "Configuring build for: TESTS only (core + tests, no frontend) due to Docker environment"
    PRESET="tests"
  else
    echo "Configuring build for: FULL (SDL, core files, and tests)"
    PRESET="default"
  fi
elif [ "$1" == "tests" ]; then
  echo "Configuring build for: TESTS only (core + tests, no frontend)"
  PRESET="tests"
else
  echo "Usage: $0 [full|tests]"
  exit 1
fi

# Navigate to the project root directory.
cd "$(dirname "$BASH_SOURCE[0]")/.." || exit 1

echo "Starting build process..."

# Create the build directory (the preset's binaryDir will be set to ${sourceDir}/${BUILD_DIR}).
echo "Creating build directory: ${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

# Run CMake configuration using the selected preset.
echo "Configuring with CMake..."
cmake --preset="$PRESET" || {
  echo "CMake configuration failed."
  exit 1
}

# Build the project.
echo "Building the project..."
cmake --build --preset="$PRESET" || {
  echo "Build failed."
  exit 1
}

# Notify the user of created executables.
echo "Checking for created executable files..."
cd "${BUILD_DIR}" || exit 1
# Use the modern -executable predicate.
executables=$(find . -maxdepth 1 -type f \( -perm -u=x -or -perm -g=x -or -perm -o=x \))

if [ -n "$executables" ]; then
  echo "Build complete. Created executables:"
  for exec in $executables; do
    echo " - ${BUILD_DIR}/$(basename "$exec")"
  done
else
  echo "No executable files were created."
fi
