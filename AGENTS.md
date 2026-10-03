# Repository Instructions

## Goal

These sources are a reconstruction of the original Stars! 2.6jrc3 C source, made
from the debug-rich Win16 `stars.exe`. The decompiler is frozen, so all work
now happens by hand in the `.c`/`.h` files. The aim is source that is as close
as possible to what the original developers wrote. It is not a modernized
rewrite.

When goals conflict, apply them in this order:

1. **Behavior:** the original game's behavior, proven by the regression and
   tutorial harnesses.
2. **Fidelity:** original names, types, statement order, control flow and
   file/function layout, as recorded in the debug info and assembly.
3. **Readability:** only where it doesn't cost 1 or 2.

## Fidelity rules

- Names that come from the debug symbols (functions, params, locals, globals,
  struct fields, enums) are original. Never rename them. Give new names only to
  decompiler artifacts (`t_scratch_*`, `t_merge_*`, `t_call_*`, `IDM_UNKNOWN_*`,
  `WMX_UNKNOWN_*`, unnamed fields), and use the original Hungarian style:
  `c` count, `i` index, `f` flag, `lp` far pointer, `rg` array, `h` handle,
  `psz`/`sz` strings, `id`/`ish`/`ipl` and the like. Check `structs.h` and
  nearby code for existing prefixes before you invent one.
- Use the original source line numbers in the asm listings (`; file.c:NNN`)
  as the guide to structure:
  - Order statements to match the line order.
  - Line gaps show where comments, blank lines or multi-line statements were.
  - Where a loop's test line sits (top or bottom) tells you `for`/`while` from
    `do`/`while`.
  - Functions in each file should appear in the order of their original line
    numbers.
- When you remove a `goto`, rebuild the structured form the original most
  likely had (`if/else`, `for`, `while`, `break`, `continue`, early `return`)
  so that it would compile under MSC to the same block layout. If no such
  structure fits, keep the `goto` with a meaningful label. Don't just hide it
  behind flags.
- Write in the original's C dialect and idiom. Declare locals at the top of a
  function, and use no C99 constructs. The exceptions are the fixed-width
  `stdint` types, `//` comments and designated initializers. Add no new
  helper functions, macros or
  abstractions, and don't restructure code into "cleaner" designs. Leave a
  repeated pattern repeated if the original repeated it.
- Write truth tests bare for flags (`f…`), `F…()` calls, pointers, handles
  and bit tests: `if (fDone)`, `if (!lpfl)`, `if (grbit & mask)`. Keep
  explicit `== 0`/`!= 0` for counts, indices, IDs and other quantities, and
  wherever the 0/1 result is used as a value. Compare pointers and handles
  with `NULL`, never `0`, when the result is used as a value.
- Don't fix original bugs in reconstructed code. A bug the original had is part
  of the source. Keep it, and list it in `HANDOFF.md` if it matters.
- Don't add explanatory comments to reconstructed code unless the original
  probably had them. Keep comments for things a reader can't see in the code.

## Marking code that is not original

Mark every native-port shim, Win16 parity repair, corruption guard and test
hook so that it can be told apart from reconstructed original code:

- `/* PARITY: ... */`: reproduces Win16 behavior for the regression. It must
  have an entry in `docs/WIN16-PARITY.md`.
- `/* NATIVE: ... */`: needed only because the build is Win32 or 64-bit
  (pointer size, `POINT16`, heap headers, CRT differences).
- `#ifdef STARS_TEST_*`: test-harness code.

Any change that emulates, guards against, or stops emulating a Win16 behavior
must update `docs/WIN16-PARITY.md` in the same change, recording where it
lives, what the original did, and what a revert should do.

## Things that must not change

- `structs.h` layouts, `WriteRt`/`ReadRt` record sizes and on-disk formats
  stay at their Win16 sizes. Native pointer or handle sizes must never reach a
  save file.
- Element widths of fields, arrays and file records stay as they are. Don't
  widen `int16_t` to `BOOL`/`int` where storage, addresses (`&f…`) or file I/O
  depend on the width. See `HANDOFF.md` §5.
- Keep the x87 rounding casts, `qsort16`, and the guards listed in
  `HANDOFF.md` §3.
- The tutorial observer depends on `InitInstance`, `ScannerWndProc`, the
  globals and struct layouts it reads, and the literal `tutor.c` signatures
  `int16_t FTutorTaskDone() {` and `int16_t FCheck`. Update the observer and
  parser in the same change if any of these move.

## Reference material

- If present, read [AGENTS-private.md](AGENTS-private.md) for additional local
  instructions. It is intentionally untracked; keep its contents and the
  materials it describes private.
- `structs.h` defines every struct. Use it to interpret offsets, bitfields and
  `LOWORD`/`HIWORD` splits.
- `reference/asm/<Func>.asm` gives each function's signature, the frame
  layout of its params and locals with BP offsets, the original source line
  numbers (`; file.c:NNN`) and the instructions.
  `reference/sem/<Func>.sem` shows the same function as semantic effects,
  which are easier to read.
  `reference/ir/<Func>.ir.c` is the function as C-like IR before
  structuring: explicit basic blocks and `goto`s. Its `L_xxxx` labels are the
  same block addresses as the labels in the `.asm`, which carry the original
  line numbers. Use the IR to map each block to its source lines and to see
  what the structurer did when you rebuild loops and conditionals.
  All three were copied from `../stars-asm/decompiled`. They are gitignored
  and frozen, so grep them freely. If they're missing, use
  `stars-asm dasm asm|sem -n <Func>`.
- Use the JSON indexes first when reconstructing or auditing control flow,
  then consult the asm/IR for the instructions and ambiguous structure:
  - `reference/lines/<Func>.json` maps instruction addresses to original
    source files and lines. Use it to place statements, jump targets and
    loop tests. `tagged: false` is an inherited line association, not a new
    explicit source-line marker.
  - `reference/labels.json` lists labels by function, their source lines and
    incoming references, including backward jumps. `debug: true` identifies
    original named labels. Use it to audit missing original labels even in
    functions with no remaining `goto L_xxxx`.
  - `reference/switches.json` records dispatch operands, case destinations,
    shared cases and default paths. A `kind: chain` entry alone does not
    establish whether the original used a `switch` or an `if/else` chain;
    check the source-line order and assembly.
  - `reference/function-lines.json` records each function's original source
    file, line range and binary location. Use it to check function order and
    source-line boundaries.
- Check the assembly before you resolve anything uncertain: fall-through
  returns, uninitialized reads, conditions that look tautological, signedness,
  evaluation order of calls, and whether a temp was a real local or a compiler
  spill.
- `HANDOFF.md` records what changed from the decompiled code, the native
  port's state, and the work left for after the `2.6jrc3` tag. Update it when
  an item is finished or found.

## Verification

Behavior-neutral cleanup must stay behavior-neutral. After each batch of
edits:

1. Build with `cmake --preset mingw-debug && cmake --build --preset mingw-debug`.
   Don't introduce new warnings in the files you touched.
2. Run a quick native regression of one scenario through checkpoint 10 and
   compare it against `tests/scaffold/starsbox/c_drive/REGTEST`. Commands are
   in `tests/scaffold/REGRESSION.md`.
3. Before a batch is considered done, run the full native suite and compare it,
   then run the tutorial (`make tutorial`). The only accepted differences are
   the three known divergences in `HANDOFF.md` §2.

Never regenerate the original DOSBox checkpoints (this takes hours) unless the
fixtures change. Report regression results as they are: if a scenario
diverges, show the first diff.

Keep commits small and focused on one kind of change, for example "fold
`t_scratch` temps in `turn2.c`". This lets a regression be bisected to a single
commit. Don't commit on the user's behalf.

## After changes

Review the diff against these rules. In particular, check for renamed original
symbols, unmarked non-original code, statement reordering that breaks
original-line order or call order, and missing `WIN16-PARITY.md` updates.
