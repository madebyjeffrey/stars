# Reconstructing the Stars! 2.6jrc3 sources

The C in this repository began as output from the stars-asm decompiler and has
since been edited by hand. It is no longer regenerated: edit the `.c`/`.h`
files directly.

The tag `2.6jrc3` marks the finished reconstruction: source as close as the
binary allows to what the original developers wrote, original bugs included.
At that tag:

- **Build:** `cmake --preset mingw-debug && cmake --build --preset mingw-debug`
  builds with no warnings. With `-Wall -Wextra -Wno-unused-parameter` there
  are 120 warnings, all kept on purpose ([NATIVE-PORT.md](NATIVE-PORT.md)).
- **Regression against the original (seed 12345):** every scenario matches at
  every checkpoint except the three known divergences in
  [REGRESSION.md](../tests/scaffold/REGRESSION.md#known-divergences).
- **Tutorial:** passes all 80 pages.
- **Release build:** `mingw-release` uses `-O3 -DNDEBUG` and strips debug data.
  The Release tutorial passes all 80 pages and the unfinished-turn rejection
  check. The six CI regression scenarios match all 306 saves through turn 150
  (58 unused-storage warnings, no differences). Published builds have all
  test hooks disabled; tutorial builds enable only the read-only observer.

This document records what changed from the decompiler's output and the
conventions the reconstruction follows. Native-port details are in
[NATIVE-PORT.md](NATIVE-PORT.md), Win16 parity repairs in
[WIN16-PARITY.md](WIN16-PARITY.md), the original release bug list in
[KNOWN-BUGS.md](KNOWN-BUGS.md), and the work after the tag in
[ROADMAP.md](ROADMAP.md).

---

## Changes from the decompiled code

Everything below is behavior-neutral: the regression and the tutorial matched
the original after each change. The guide throughout was the binary: the
debug-info names, the original source line numbers in the asm, and the
labels, switches and IR under `reference/`.

**Control flow**

- **`goto`s:** every decompiler `goto L_xxxx` is gone, rebuilt as the
  `if`/`else`, loop, `switch`, `break`/`continue` or early `return` the line
  data shows. Every original named label from the debug info is restored at
  its recorded line, with a `goto` at each recorded jump source.
  `tests/scaffold/labaudit.py` checks this.
- **Block order:** statements follow the original source-line order. The
  decompiler had often inverted conditions (putting the original `else` arm
  first) and moved shared error and exit blocks to the end of a function. Those
  are back where the original had them, with later code jumping back to them.
- **`switch` vs `if`:** where the asm dispatches after the case bodies, the
  code is a `switch`. Where it tests inline, it is an `if` chain. Case order
  follows the original lines. Jump-table filler cases and a hoisted
  jump-table range check were removed.
- **Message dispatch:** each window and dialog procedure handles its messages
  in a single `switch`, in original order. `WM_CTLCOLOR` goes through a
  `NATIVE` shim; see [WIN16-PARITY.md](WIN16-PARITY.md).
- **Loops:** `for`, `while` and `do` forms follow the loop-test placement.
  The remaining `while (1)` loops are genuine infinite loops with mid-body
  exits. `while (1)`, `while (TRUE)` and `for (;;)` compile identically, so
  the binary cannot tell which spelling the original used.

**Expressions and names**

- **Temps:** all decompiler temps (`t_scratch_*`, `t_merge_*`, `t_call_*`,
  post-increment temps) are folded back into the expressions they came from.
  This restores the `windows.h` `min`/`max` macros and the chained and
  embedded assignments. `DGetDistance`'s `__fac` store, which was MSC's
  floating-point return convention, is removed. The decompiler had truncated
  one 32-bit add in `CostOfDevelopingItem` through a 16-bit temp; it is 32-bit
  again.
- **Truth tests:** flags, `F…()` calls, pointers, handles and bit tests use
  bare tests (`if (fDone)`, `if (!lpfl)`), loop conditions included. Where a
  pointer comparison's 0/1 result is stored, it compares with `NULL`. Counts,
  indices and IDs keep explicit `== 0`/`!= 0`. The compiled code is
  unchanged.
- **`TRUE`/`FALSE`:** used for genuine boolean stores and success/failure
  returns. [Storage widths and boolean conventions](#storage-widths-and-boolean-conventions)
  lists the `f…` names that hold other values.
- **Named constants:** literals were replaced by enum constants where the
  meaning is clear: part IDs, report columns, production status
  (`mdProdStat`), part availability (`mdPartAvail`), 0xffff and -1 sentinels
  (`iplrNone`, `idPlanetNone`, `ishdefNone` and so on), and the handler IDs
  that had been `IDM_UNKNOWN_*`. Quantities, relational tests and generic
  polymorphic IDs keep their numbers.
- **`LOWORD`/`HIWORD`:** audited. Packed messages and extents, record words
  and intentional numeric narrowing are kept.

**Left as is, by decision**

- **Block-scoped locals:** the debug info records locals declared in inner
  blocks (209 functions). They stay declared at the top of each function.
- **Return shape:** some procedures `return` from each case where the original
  `break`s to a shared `return 0` (for example `FrameWndProc` and
  `PopupWndProc`).
- **Not recoverable from the binary:** macros the compiler expanded,
  comments, formatting, and exact line-for-line layout. Same-line argument
  order follows MSC's right-to-left evaluation. `SetAiFleetIdealSpeed`'s
  `case 0:` has no code of its own, so its original position is unknown.

---

## Storage widths and boolean conventions

- **Widths:** fields, arrays, file records and addressed locals keep their
  original widths. Scalars are not declared `BOOL`, because Win32 `BOOL` is 32
  bits. The `int16_t *` parameters `pfMulti`, `pfTwo`, `pfProgress` and
  `pfDampeningField` match their callers' 16-bit locals. `rgfBmpUsed` holds
  words, while `rgfNoXFile`, `rgfSeen`, `rgfCheater`, `rgfTorp` and `rgfInit`
  hold bytes.
- **Not 0/1:** these `f…` names hold states, masks or IDs, so they don't take
  `TRUE`/`FALSE`:
  - **Locals:** `WriteBattles.fPlayerCur`, `DrawVCR.fJam`,
    `DrawScanner.fPlanetScanner`/`fStarbase`,
    `SatisfyOrders.fMining`/`fDunnage`/`fHasPermission`,
    `CBuildProdItem.fAutoBuild`, `UpdateResearchStatus.fGeneral`,
    `IdTargetFreighter.fNeedy`, `TbWndProc.fCur`, `FReadShDef.fOkay`, and the
    dialog-result `fRet`/`fSuccess` locals.
  - **Parameters:** `_Draw3dFrame.fErase`, `RedrawScanSel.fVis`,
    `ChangeScanSel.fValidScan`, `DrawProductionItem.fListbox`,
    `InitScoreDlg.fVictory`, `InvalidateReport.fReload`,
    `FCheckQueue.fNoResearch`, and the `fkb` key masks.
  - **Returns:** `FLookupPart`, `FLookupPartX`, `FOpenGame`, `FWasRaceFile`,
    `FCanFleetUseStargates`, `FDoCoolBattle` and `FWriteTutorialMFile`.
- **Numeric success returns:** `InitInstance`, `InitMDIApp`, `ReadBigBlock`,
  `GenerateWorld`, `GenNewGameFromFile`, `GetVCCheck`, `ChangeProduction`,
  `RaceCreationWizard` and `GetRaceGrbit` are success/failure predicates.
  `DrawScanner`, `TransferStuff` and `ShipBuilder` always return 0, and
  `DxyMoveTokTo` always returns 1. Their returns stay numeric.
