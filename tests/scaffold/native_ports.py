#!/usr/bin/env python3
"""Link focused native-port checks against the actual built game objects."""

import argparse
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", type=Path, default=ROOT / "dist/mingw-debug")
    args = parser.parse_args()
    objects = sorted((args.build.resolve() / "CMakeFiles/stars.dir").glob("*.c.obj"))
    if not objects:
        parser.error("build the game with the mingw-debug preset first")
    output = ROOT / "dist/scaffold/native-ports"
    output.mkdir(parents=True, exist_ok=True)
    exe = output / "native-ports.exe"
    wraps = ("WriteRt", "FCreateFile", "StreamOpen", "StreamClose", "DirtyGame", "ReadRt")
    subprocess.run([
        "x86_64-w64-mingw32-gcc", "-DSTARS_TEST_NATIVE_PORTS", "-I", str(ROOT),
        "-Wno-unused-parameter", "-Wno-pointer-sign", "-fsigned-char",
        str(ROOT / "tests/scaffold/native_ports.c"), *map(str, objects),
        "-Wl," + ",".join("--wrap=" + name for name in wraps),
        "-luser32", "-lgdi32", "-lcomdlg32", "-o", str(exe),
    ], check=True)
    subprocess.run(["wine", str(exe)], check=True)


if __name__ == "__main__":
    main()
