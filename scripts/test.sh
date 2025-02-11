#!/bin/bash
# scripts/test.sh
# Optional test script to run CTest in the build directory
# Give file permission to execute: chmod +x scripts/test.sh
# Run script: ./scripts/test.sh
BUILD_DIR="build"

cd "$(dirname "$0")/.." || exit 1 # run from the project root directory

# Navigate to the build directory and run ctest
cd "$BUILD_DIR" || {
  echo "Error: Could not enter build directory '$BUILD_DIR'."
  exit 1
}

# Run specific test if provided, otherwise run all tests
if [ -n "$1" ]; then
  echo "Running specific test: $1"
  ctest -R "$1" -V # Matches test names against provided pattern
else
  echo "Running all tests"
  ctest
fi

# Isolating a specific test locally
# ./scripts/test.sh "CPUTestFixture.IMM" # Immediate addressing test
# ./scripts/test.sh "CPUTestFixture.xEA" # Test for opcode 0xEA (JMP)

