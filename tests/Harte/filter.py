import json


to_filter = {
    "61_decimal.json": "61.json",
    "65_decimal.json": "65.json",
    "69_decimal.json": "69.json",
    "6d_decimal.json": "6d.json",
    "71_decimal.json": "71.json",
    "75_decimal.json": "75.json",
    "79_decimal.json": "79.json",
    "7d_decimal.json": "7d.json",
    "e1_decimal.json": "e1.json",
    "e5_decimal.json": "e5.json",
    "e9_decimal.json": "e9.json",
    "ed_decimal.json": "ed.json",
    "f1_decimal.json": "f1.json",
    "f5_decimal.json": "f5.json",
    "f9_decimal.json": "f9.json",
    "fd_decimal.json": "fd.json",
}

for in_file, out_file in to_filter.items():
    with open(in_file, "r") as f:
        tests = json.load(f)
    filtered_tests = [test for test in tests if (test["initial"]["p"] & 8) == 0]
    with open(out_file, "w") as f:
        f.write("[\n")
        f.write(",\n".join(json.dumps(test) for test in filtered_tests))
        f.write("\n]")
