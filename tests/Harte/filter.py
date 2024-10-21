import json

# Load the JSON file
with open("61_decimal.json", "r") as f:
    tests = json.load(f)

# Filter out tests where the Decimal flag (bit 3) is set in the initial 'p'
filtered_tests = [test for test in tests if (test["initial"]["p"] & 8) == 0]

# Write the filtered JSON into an array, with one item per line
with open("61.json", "w") as f:
    f.write("[\n")
    f.write(",\n".join(json.dumps(test) for test in filtered_tests))
    f.write("\n]")
