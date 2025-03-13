
#include "bus.h"
#include <fmt/base.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

class Emulator
{
  public:
    Bus        bus;
    PPU       &ppu = bus.ppu;
    CPU       &cpu = bus.cpu;
    Cartridge &cart = bus.cartridge;

    Emulator() = default;

    // CPU Getters
    u64 GetCycles() const { return bus.cpu.GetCycles(); }
    u8  A() const { return cpu.GetAccumulator(); }
    u8  X() const { return cpu.GetXRegister(); }
    u8  Y() const { return cpu.GetYRegister(); }
    u8  SP() const { return cpu.GetStackPointer(); }
    u16 PC() const { return cpu.GetProgramCounter(); }
    u8  P() const { return cpu.GetStatusRegister(); }

    u8 CarryFlag() const { return cpu.GetCarryFlag(); }
    u8 ZeroFlag() const { return cpu.GetZeroFlag(); }
    u8 InterruptFlag() const { return cpu.GetInterruptDisableFlag(); }
    u8 DecimalFlag() const { return cpu.GetDecimalFlag(); }
    u8 BreakFlag() const { return cpu.GetBreakFlag(); }
    u8 OverflowFlag() const { return cpu.GetOverflowFlag(); }
    u8 NegativeFlag() const { return cpu.GetNegativeFlag(); }

    // PPU Getters
    u8 GetNmi() const { return ppu.GetCtrlNmiEnable(); }
    u8 GetVblank() const { return ppu.GetStatusVblank(); }

    // Methods
    void Load( const std::string &path )
    {
        bus.cartridge.LoadRom( path );
        bus.cpu.Reset();
    }

    void Preset()
    {
        // Loads a common rom used for debugging to get going quickly.
        std::string romFile = "../../tests/roms/custom.nes";
        bus.cartridge.LoadRom( romFile );
        bus.cpu.Reset();
    }

    static void Test() { fmt::print( "Test\n" ); }
    void        Log()
    {
        auto out = bus.cpu.LogLineAtPC();
        fmt::print( "{}\n", out );
    }

    void Step( int n = 1 )
    {
        for ( int i = 0; i < n; i++ ) {
            cpu.DecodeExecute();
        }
    }

    void EnableMesenTrace( int n = 100 )
    {
        cpu.EnableMesenFormatTraceLog();
        cpu.SetMesenTraceSize( n );
    }
    void DisableMesenTrace() { cpu.DisableMesenFormatTraceLog(); }
    void PrintMesenTrace() const
    {
        for ( const auto &line : cpu.GetMesenFormatTracelog() ) {
            fmt::print( "{}", line );
        }
    }
};

PYBIND11_MODULE( emu, m )
{
    py::class_<Emulator>( m, "Emulator" )
        .def( py::init<>() )
        .def_property_readonly( "cpu_cycles", &Emulator::GetCycles, "Get the number of cycles" )
        .def_property_readonly( "a", &Emulator::A, "Get the accumulator" )
        .def_property_readonly( "x", &Emulator::X, "Get the X register" )
        .def_property_readonly( "y", &Emulator::Y, "Get the Y register" )
        .def_property_readonly( "sp", &Emulator::SP, "Get the stack pointer" )
        .def_property_readonly( "pc", &Emulator::PC, "Get the program counter" )
        .def_property_readonly( "p", &Emulator::P, "Get the status register" )
        .def_property_readonly( "carry_flag", &Emulator::CarryFlag, "Get the carry flag" )
        .def_property_readonly( "zero_flag", &Emulator::ZeroFlag, "Get the zero flag" )
        .def_property_readonly( "interrupt_flag", &Emulator::InterruptFlag, "Get the interrupt flag" )
        .def_property_readonly( "decimal_flag", &Emulator::DecimalFlag, "Get the decimal flag" )
        .def_property_readonly( "break_flag", &Emulator::BreakFlag, "Get the break flag" )
        .def_property_readonly( "overflow_flag", &Emulator::OverflowFlag, "Get the overflow flag" )
        .def_property_readonly( "negative_flag", &Emulator::NegativeFlag, "Get the negative flag" )
        .def_property_readonly( "get_nmi", &Emulator::GetNmi, "Get the PPU NMI flag" )
        .def_property_readonly( "get_vblank", &Emulator::GetVblank, "Get the PPU VBLANK flag" )
        .def( "load", &Emulator::Load, "Load a rom file" )
        .def( "preset", &Emulator::Preset, "Load custom.nes rom for debugging" )
        .def( "log", &Emulator::Log, "Log CPU state" )
        .def( "step", &Emulator::Step, "Step the CPU by one or more cycles", py::arg( "n" ) = 1 )
        .def( "enable_mesen_trace", &Emulator::EnableMesenTrace, "Enable Mesen trace log",
              py::arg( "n" ) = 100 )
        .def( "disable_mesen_trace", &Emulator::DisableMesenTrace, "Disable Mesen trace log" )
        .def( "print_mesen_trace", &Emulator::PrintMesenTrace, "Print Mesen trace log" )
        .def_static( "test", &Emulator::Test, "Test function" );
}
