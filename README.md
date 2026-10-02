# Stars!
![meme](docs/images/windows10stars.jpg)

Stars! 4X game rebuilt from decompiled C sources using a custom [stars-asm](https://github.com/sirgwain/stars-asm/tree/main) win16 disassembler the decompiler.

The original 2.6jrc3 stars.exe included ~1MB of debug symbols with function names, variable names, symbol definitions, and even line numbers. The stars-asm project used that information to rebuild the Stars! source to be as close to the original as possible. That is what is in this repo.

## Build

The application uses Windows APIs. On macOS or Linux, build a Windows executable
with CMake 3.23 or later, Ninja, and the x86_64 MinGW-w64 toolchain on PATH.
On macOS these build dependencies can be installed with Homebrew:

```sh
brew install cmake ninja mingw-w64
```

From the project directory:

```sh
cmake --preset mingw-debug
cmake --build --preset mingw-debug
```

The executable is written to `dist/mingw-debug/bin/stars.exe`. CMake also compiles the
resources in `res/` and tracks their embedded files for rebuilds.

Wine is optional for building. If `wine` is on PATH when configuring, run with:

```sh
cmake --build --preset run-wine
```

For a release build:

```sh
cmake -S . -B dist/mingw-release -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=toolchains/mingw-w64.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build dist/mingw-release
```

## Regression save tooling

The standalone go module in `tests/savecli/` supplies the save
comparison and AI update commands used by checkpoint testing:

```sh
make save-cli
make checkpoints-compare
```

Checkpoint targets build `dist/stars-save` automatically. They require a local
starsbox bundle at `tests/scaffold/starsbox`. See
[the save CLI README](tests/savecli/README.md) and
[regression instructions](tests/scaffold/REGRESSION.md).
