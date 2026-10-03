# Unit tests

Each `test_<name>.c` here is an [acutest](https://github.com/mity/acutest)
program linked with the game code (the `stars_core` object library) and its
resources. They build as Windows console programs with the normal MinGW
build, and CTest runs them through Wine:

```sh
make test-unit                           # build mingw-debug and run them all
make test-unit CTEST_ARGS='-R test_turn' # one file
dist/mingw-debug/tests/test_turn.exe --list   # under wine: the tests in a file
```

Acutest runs every test in its own process, so each test starts from the
game's initial globals. Tests run in `<build>/tests`, where CMake copies the
fixture race to `data/humanoid.r1`.

## Two kinds of test

- **Function tests** call one game function with crafted inputs, such as
  `FIntersectCircleLine` in `test_utilgen.c`.
- **Turn tests** build a game and generate turns with the helpers in
  `stars_test.h`: `FStarsTestInit`, `FStarsTestDir` (a fresh
  `work/<name>` directory), `FStarsTestNewGame` (a tiny universe with the
  test race and any AI players), `FStarsTestLoadHost` and
  `FStarsTestGenerate`. Between loading the host and generating, a test can
  change the loaded game (`LpplStarsTestHomeworld`, `LpflStarsTestAddFleet`,
  or directly) and save it with `FStarsTestSaveHost`. AI players give their
  orders while one turn generates and the next turn carries them out, so AI
  behavior needs two generations. `test_turn.c` shows the plumbing and
  `test_ai4.c` a full example.

## Bug fixes

A bug fix in [docs/ROADMAP.md](../../docs/ROADMAP.md) step 5 adds a test that
fails on the code before the fix. Name the test after the behavior it
checks and put it in the file for the source file that holds the fix.
`test_native_ports.c` links with `--wrap` for file I/O (see
`CMakeLists.txt`); give other tests that need wraps the same treatment.
