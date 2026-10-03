#!/usr/bin/env python3
"""Audit labels and gotos in reconstructed C against the binary's label index."""

import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[2]
LABELS = ROOT / "reference/labels.json"
FUNC = re.compile(r"^[A-Za-z_][\w \*]*?\b(\w+)\(")
LABEL = re.compile(r"^\s*([A-Za-z_]\w*):\s*;?\s*$")
GOTO = re.compile(r"\bgoto (\w+);")
NOT_FUNC = ("return", "if", "else", "switch", "case", "typedef")


def scan(path):
    """scan maps each function in a C file to its label definitions and goto counts."""
    funcs = {}
    fn = None
    for line in path.read_text().splitlines():
        match = FUNC.match(line)
        if match and not line.rstrip().endswith(";") and not line.startswith(NOT_FUNC):
            fn = match[1]
            funcs[fn] = {"labels": set(), "gotos": {}}
        if fn is None:
            continue
        label = LABEL.match(line)
        if label and label[1] != "default":
            funcs[fn]["labels"].add(label[1])
        for target in GOTO.findall(line):
            funcs[fn]["gotos"][target] = funcs[fn]["gotos"].get(target, 0) + 1
    return funcs


def audit(path, index):
    """audit returns the findings for one C file."""
    findings = []
    for fn, seen in scan(path).items():
        entries = {entry["name"]: entry for entry in index.get(fn, [])}
        debug = {name for name, entry in entries.items() if entry["debug"]}
        for name in sorted(seen["labels"]):
            if not name.startswith("L_") and name not in debug:
                findings.append(f"{path.name}:{fn}: label {name} is not in the debug info")
        for name in sorted(debug - seen["labels"]):
            refs = entries[name]["refs"]
            line = entries[name]["src"]["line"]
            if any(ref["backward"] for ref in refs):
                findings.append(f"{path.name}:{fn}: debug label {name} (line {line}) has backward jumps but is missing")
            elif refs:
                findings.append(f"{path.name}:{fn}: debug label {name} (line {line}) is jumped to but is missing")
        for name, count in sorted(seen["gotos"].items()):
            if name.startswith("L_") or name not in entries:
                continue
            sources = {ref["src"]["line"] for ref in entries[name]["refs"] if ref["kind"] != "table"}
            if count > len(sources):
                findings.append(f"{path.name}:{fn}: {count} gotos to {name} but {len(sources)} recorded jump source lines")
    return findings


def main():
    """main audits each C file named on the command line, or every C file in the repository."""
    if not LABELS.exists():
        sys.exit(f"{LABELS} is missing; copy the stars-asm exports into reference/")
    index = json.loads(LABELS.read_text())
    paths = [Path(arg) for arg in sys.argv[1:]] or sorted(ROOT.glob("*.c"))
    findings = [finding for path in paths for finding in audit(path, index)]
    print("\n".join(findings) if findings else "no findings")
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
