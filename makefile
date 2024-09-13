CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude

all: emu

emu: main.o cpu.o
	@echo "Linking..."
	g++ $(CXXFLAGS) main.o cpu.o -o emu

main.o: src/main.cpp
	@echo "Compiling main.cpp..."
	g++ $(CXXFLAGS) -c src/main.cpp

cpu.o: src/cpu.cpp
	@echo "Compiling cpu.cpp..."
	g++ $(CXXFLAGS) -c src/cpu.cpp

clean:
	@echo "Removing everything but the source files"
	rm -f *.o emu
