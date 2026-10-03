# Stars!

![meme](docs/images/windows10stars.jpg)

Stars! 4X game rebuilt from decompiled C sources using a custom [stars-asm](https://github.com/sirgwain/stars-asm/tree/main) win16 disassembler the decompiler.

The original 2.6jrc3 stars.exe included ~1MB of debug symbols with function names, variable names, symbol definitions, and even line numbers. The stars-asm project used that information to rebuild the Stars! source to be as close to the original as possible.

## Branches

- `2.6j` (tag `2.6jrc3`): the faithful reconstruction, original bugs
  included. It matches the original game turn for turn in the fixed-seed
  regression.
- `main`: the 2.8 line. It fixes original bugs and replaces the Win16 shims
  with native code, while keeping the 2.6j file formats.

## Documentation

- [Reconstruction](docs/RECONSTRUCTION.md): how the source was rebuilt from the
  decompiler's output, and the conventions it follows.
- [Native port](docs/NATIVE-PORT.md) and [Win16 parity](docs/WIN16-PARITY.md):
  what the Win32/Win64 build changes, and the original behavior it reproduces.
- [Known bugs](docs/KNOWN-BUGS.md): the original release bug list, mapped to
  source.
- [Roadmap](docs/ROADMAP.md): the work planned for 2.8.

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
cmake --preset mingw-release
cmake --build --preset mingw-release
```

This writes `dist/mingw-release/bin/stars.exe` with optimization enabled and
debug data stripped. Test hooks are disabled in ordinary builds.

## GitHub Actions

Every push to `main` builds the optimized MinGW Release executable and updates
the rolling [`latest` prerelease](https://github.com/sirgwain/stars/releases/tag/latest).
Both `stars.exe` and `stars!.hlp` are attached; download them into the same
directory. Superseded main builds remain available as workflow artifacts.

Pushing a tag (for example `2.6jrc3`) builds that tag's source and publishes a
release with the same two files. The rolling `latest` tag is excluded. To test
the tagged release, include the workflows and `res/stars!.hlp` in the tagged
commit, then push the tag:

```sh
git tag 2.6jrc3
git push origin 2.6jrc3
```

Pull requests build and run the complete tutorial and the unfinished-turn
rejection check under Wine/Xvfb. Diagnostic reports are retained even on failure.
The tutorial uses the same Release preset as the published builds, with the
read-only test observer enabled. The native regression workflow also builds in
Release mode on pull requests and main pushes, comparing
all checkpoints through turn 150 against the checked-in native baseline
(`tests/scaffold/fixtures/regression/native/`). The original game's
checkpoints are kept alongside it as the record of 2.6j behavior.
All workflows can also be run manually; the release workflow accepts `main`
or a tag. Publishing uses the built-in `GITHUB_TOKEN` with `contents: write`;
the tutorial uses read-only permissions. The optional repository variable
`STARS_TUTORIAL_SERIAL` overrides the tutorial runner's default serial.

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
