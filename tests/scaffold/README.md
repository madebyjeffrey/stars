# Game test scaffolding

The scaffolding covers new-game generation, fixed-seed regression comparisons
against the original game, and the tutorial UI under Wine.

- [Regression guide](REGRESSION.md): scenarios `noai`, `oneai1`–`oneai6`,
  `smallai4`, and `smallai6`, with checkpoints at turns 0, 1, 10, 25, 50, 80,
  100, and 150.
- [Tutorial guide](tutorial/README.md): the full walkthrough and premature
  Generate rejection, driven by AutoHotkey v2.0.28.
- [Save CLI](../savecli/README.md): the standalone Go/Cobra save comparison and
  AI update commands. No stars-asm binary or checkout is required.

## Layout

```text
tests/scaffold/
  newgame.sh                     New-game smoke test runner
  regression.py                  Checkpoint generation, comparison, and tracing
  regression_trace.c             Optional native trace wrappers
  seed_exe.py                    Fixed-seed patcher for a copy of the original
  fixtures/newgame/tiny/         Single-player game.def and humanoid.r1
  fixtures/regression/           Fixed-seed AI scenario definitions and report
  tutorial/                     AutoHotkey runner, scripts, and observer
  starsbox/                     Local DOSBox bundle and reference saves (ignored)

tests/savecli/                  Standalone save CLI module

dist/                          Generated builds, tools, and run artifacts (ignored)
  stars-save                    Save CLI executable
  scaffold/newgame/run/         Recreated by the default smoke-test run
  scaffold/tutorial/            Retained tutorial results by timestamp
```

Keep fixture inputs in `fixtures/`. The Makefile's checkpoint runs default to
`tests/scaffold/starsbox/c_drive/REGTEST` for the original and
`tests/scaffold/starsbox/c_drive/native` for native saves. Preserve the reference
checkpoints: regenerating them takes many hours.

## Regression and tutorial commands

Run from the repository root:

```sh
make compile
make checkpoints-compare
make tutorial
make tutorial-reject
```

`checkpoints-compare` builds `dist/stars-save` automatically and compares existing
runs. Differences produce a nonzero exit status, including the known baseline
divergences recorded in [REGRESSION.md](REGRESSION.md#known-divergences) and the
regression report.

`make checkpoints-native` builds with a fixed seed and replaces the native run.
`make checkpoints-starsbox` replaces the original run and can take many hours;
use it only when intentionally regenerating the reference. Both commands delete
their configured work directory before staging a fresh run.

The tutorial targets build with `STARS_TEST_TUTORIAL=ON` and use an isolated Wine
prefix. They enter the configured serial through the normal dialog when needed.
See the tutorial guide for `STARS_TUTORIAL_SERIAL`, registration INI files,
retained results, and runtime requirements.

## New-game smoke test

Build the executable, then invoke the script directly:

```sh
make compile
tests/scaffold/newgame.sh
```

There is no `make newgame` target. The script requires Bash, Perl, and Wine on
`PATH`. It uses Wine's default prefix (or `WINEPREFIX` if set). That prefix's
`Stars.ini` must contain a registered serial; command-line creation with `-a`
does not register the game interactively.

To select a fixture, executable, timeout, or output directory:

```sh
tests/scaffold/newgame.sh --exe dist/mingw-debug/bin/stars.exe \
  --timeout 60 --work dist/scaffold/newgame/custom \
  tests/scaffold/fixtures/newgame/tiny/game.def
```

The work directory is deleted and recreated on every run. Defaults resolve from
the repository location; explicit relative paths resolve from the current working
directory. Use a dedicated scratch directory.

Each fixture has a `game.def` and its race files. Race paths are relative to the
fixture directory, and output paths are relative to the run directory. Use
forward slashes and relative paths; the runner converts separators for Stars!.

The smoke test passes when the executable exits successfully within the timeout
and writes the `.xy` universe and `.hst` host files named by the definition.
It retains staged inputs, generated saves, and `wine.log` for diagnosis. An error
dialog can block until the timeout. This smoke test checks file generation;
content comparison and turn processing belong to the regression harness.
