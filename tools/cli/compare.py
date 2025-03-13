import re

pattern = re.compile(r'^([A-F0-9]{4}).*?:\s*([A-F0-9]{2}).*?:\s*([A-F0-9]{2}).*?:\s*([A-F0-9]{2}).*?:\s*([A-F0-9]{2}).*?:\s*[A-F0-9]*\s*(.{8}).*?:\s*(\d+).*?:\s*(\d+).*?:\s*(\d+)')

def parse_line_type1(line):
    """
    Parse a line from out.txt (type1 format) and return a dictionary
    with the relevant fields.
    """
    m = pattern.match(line)
    if m:
        return {f"group{i+1}": group for i, group in enumerate(m.groups())}
    return None

def parse_line_type2(line):
    """
    Parse a line from mesen.txt (type2 format) and return a dictionary
    with the relevant fields.
    """
    m = pattern.match(line)
    if m:
        return {f"group{i+1}": group for i, group in enumerate(m.groups())}
    return None

def main():
    # Open both files
    with open("out.txt", "r") as file1, open("mesen.txt", "r") as file2:
        for line_num, (line1, line2) in enumerate(zip(file1, file2), 1):
            parsed1 = parse_line_type1(line1)
            parsed2 = parse_line_type2(line2)
            
            # Compare the parsed fields.
            if parsed1 != parsed2:
                print(f"Difference found at line {line_num}:")
                print("A:", [val for val in parsed1.values() if val])
                print("B:", [val for val in parsed2.values() if val])

                print("")
                print("Original lines:")
                print(f"out.txt:   {line1.strip()}")
                print(f"mesen.txt: {line2.strip()}")
                break

if __name__ == "__main__":
    main()
