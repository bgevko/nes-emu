#!/bin/bash
# test.sh

BUILD_DIR="build"

# Check if the build directory exists
if [ ! -d "$BUILD_DIR" ]; then
	echo "Error: Build directory '$BUILD_DIR' does not exist. Please run ./build.sh first."
	exit 1
fi

# Navigate to the build directory
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
	ctest -V
fi

# Isolating a specific test
# ./test.sh "CPUTest.IMM" # Immediate addressing test
# ./test.sh "CPUTest.xEA" # Test for opcode 0xEA (JMP)
