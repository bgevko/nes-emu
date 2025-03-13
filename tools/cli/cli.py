import sys
import os
import emu

# ANSI escape codes for colors
BLACK   = "\033[30m"
RED     = "\033[31m"
GREEN   = "\033[32m"
YELLOW  = "\033[33m"
BLUE    = "\033[34m"
MAGENTA = "\033[35m"
CYAN    = "\033[36m"
WHITE   = "\033[37m"
RESET   = "\033[0m"

def print_interactive(msg):
    """Prints messages to stderr so they are not captured when piping stdout."""
    print(msg, file=sys.stderr)

e = emu.Emulator()
e.preset()
e.enable_mesen_trace(100000)

# Only show interactive messages if running in a tty.
if sys.stdout.isatty():
    print_interactive(f"{GREEN}e = emu.Emulator(){RESET}")
    print_interactive(f"{BLUE}Loading ROM: tools/asm/custom.nes{RESET}")
    print_interactive(f"{GREEN}e.preset(){RESET}")
    print_interactive(f"{CYAN}Done{RESET}")
    print_interactive(f"{YELLOW}Use 'e' to access the emulator object{RESET}")
    print_interactive(f"For available commands, type {YELLOW}'commands()'{RESET}")

def bind_method(method_name):
    def wrapper(*args, **kwargs):
        attr = getattr(e, method_name)
        if callable(attr):
            return attr(*args, **kwargs)
        else:
            return attr
    wrapper.__name__ = method_name
    return wrapper

# Names exposed to the interactive shell
method_names = [
    "cpu_cycles", 
    "a", "x", "y", "p", "sp", "pc",
    "carry_flag", "zero_flag", "interrupt_flag", "decimal_flag", "break_flag", "overflow_flag", "negative_flag",
    "log", "step", "test", "get_nmi", "get_vblank", "enable_mesen_trace", "disable_mesen_trace", "print_mesen_trace"
]
for name in method_names:
    globals()[name] = bind_method(name)

def step_until(callback_condition):
    while not callback_condition():
        e.step()

def commands():
    for name in method_names:
        print(f"{YELLOW}{name}(){RESET}")
    print(f"{YELLOW}step_until(callback_condition){RESET}")
    print(f"{YELLOW}out(filename){RESET} # Redirect stdout to a file")

def step_and_trace(n = 100):
    e.step(n)
    e.print_mesen_trace()

def main():
    # step_and_trace(100000)
    step(25714)
    pass
if __name__ == "__main__":
    main()
