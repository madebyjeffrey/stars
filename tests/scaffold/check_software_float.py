#!/usr/bin/env python3
"""Reject native x86 floating arithmetic in compiled game and UI objects.

Native float/double registers may carry bits (moves are allowed), but math,
comparisons and numeric conversions must call the software implementation.
This checks the game objects, not CRT libraries or test-only native oracles.
"""
import argparse
from pathlib import Path
import re
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--build", type=Path, required=True)
parser.add_argument("--objdump", default="objdump")
args = parser.parse_args()
objects = sorted(p for layer in ("core", "ui")
                 for p in (args.build / "CMakeFiles" / f"stars_{layer}.dir").rglob("*")
                 if p.suffix in (".o", ".obj"))
if not objects:
    raise SystemExit("No compiled game objects found")
bad = []
for obj in objects:
    data = subprocess.check_output([args.objdump, "-d", str(obj)], text=True)
    if not re.search(r"file format (?:pe|elf)(?:i)?(?:-x86-64|64-x86-64|32-i386)", data):
        raise SystemExit(f"Unsupported architecture (this audit is for x86): {obj}")
    for line in data.splitlines():
        match = re.match(r"\s*[0-9a-f]+:\s+(?:[0-9a-f]{2}\s+)+\s*([a-z0-9]+)\b", line)
        if not match:
            continue
        op = match[1]
        if (re.fullmatch(r"v?(?:(?:add|sub|mul|div|sqrt|min|max|round)(?:ss|sd|ps|pd)|"
                         r"(?:u?comi)(?:ss|sd)|cvt\w+|fm(?:add|sub)\w+)", op)
                or re.match(r"f(?:ld|st|add|sub|mul|div|com|ucom|ist|ild|sqrt|sin|cos|ptan|patan|scale|rndint|2xm1|yl2x)", op)):
            bad.append(f"{obj}: {line.strip()}")
print(f"{len(objects)} game/UI objects; {len(bad)} native floating instructions")
if bad:
    print("\n".join(bad))
    raise SystemExit(1)
