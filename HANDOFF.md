# Handoff: the reconstructed Stars! 2.6jrc3 sources

The C in this repository began as output from the stars-asm decompiler and has
since been edited by hand. It is no longer regenerated: edit the `.c`/`.h`
files directly.

Baseline:

- **Build:** `cmake --preset mingw-debug && cmake --build --preset mingw-debug`
  builds with no warnings. With `-Wall -Wextra -Wno-unused-parameter` there
  are 120 warnings, all kept on purpose (§4).
- **Regression against the original (seed 12345):** every scenario matches at
  every checkpoint except the three divergences in §2.
- **Tutorial:** passes all 80 pages.

The work has two phases. The source cleanup (§1) is complete, and its result
is tagged `2.6jrc3`: the faithful reconstruction, original bugs included.
Bug fixing (§6) starts only after that tag.

This document records what changed from the decompiler's output, the native
port's state, and the work left for after the tag. Native-port shims and
Win16 parity repairs are described in detail in `docs/WIN16-PARITY.md`. The
original release bug list, mapped to source, is in `docs/known-bugs.md`.

---

## 1. Changes from the decompiled code

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
  `NATIVE` shim; see `docs/WIN16-PARITY.md`.
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
  returns. §5 lists the `f…` names that hold other values.
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

## 2. Regression divergences

These are the only mismatches with the original. Each comes from the original
reading uninitialized stack memory, so the original value can't be reproduced
deterministically. They stay until after the tag, then get fixed (§6).

| Scenario | First diff | Cause | Where |
| --- | --- | --- | --- |
| oneai5 | t80 checkpoint, `rtOrderA tlm.cTime 0xa2e2 → 0x0001` | The Cybertron mine-laying `ORDER ord` never sets its task union. At turn 80 or earlier the slot holds stack residue that changes with the save path's length. | `ai4.c` `DoCyberAiTurn`, `rgbOrdFrame` |
| smallai6 | t57 (t80 checkpoint) | `TargetMacArmada` compares an uninitialized `cshWar`. `FPotentMacWarFleet` returns without writing `*pcEquiv` for weak fleets. | `ai3.c` `TargetMacArmada`, `FPotentMacWarFleet` |
| oneai6 | t68 (t80 checkpoint) | Same `cshWar` cause. With `cshWar = 1000` it matches to t84, then the RNG draw count drifts. | same |

The oneai5 task-union words depend on the save path, so they can differ from
the committed report in runs from other directories. For bisecting, use
`regression.py crossfeed`/`bisect`, a `-DSTARS_TEST_TRACE=ON` build with
`STARS_TRACE=trace.log`, and the `.xN` AI logs in each save directory.

---

## 3. Win16 parity sites

These sites reproduce original bugs so the regression stays exact. Revert them
after the tag (§6), and update `docs/WIN16-PARITY.md` with each revert.

| Site | Parity behavior | Revert to |
| --- | --- | --- |
| `ai3.c` `DoMacintiAiTurn` | `iLatestMiner != -1 ? rgshdef[iLatestMiner].cExist : (vtimer.mdForce \| vtimer.fAutoGenWhenIn << 16)` | Treat -1 as "no design" (count 0) |
| `ai3.c` `DoMacintiAiTurn`, `ai.c` `DoRobotoidAiTurn` | `iLatestDestroyer != -1 ? lpfl->rgcsh[...] : lpfl->pt.y` | Treat -1 as count 0, or skip the test |
| `ai4.c` `DoCyberAiTurn` `rgbOrdFrame[150]` | `shdef`, `ord` (+0x84) and `rgRecycleSBShdef` (+0x86) overlaid like the Win16 frame | Separate locals again, with `ord.tlm.cTime = 5; ord.tlm.cTimeOld = 5;` as in the AI's other mine-laying orders |
| `ai3.c` `DoMacintiAiTurn` mine-laying order | Task union never set (not reproducible) | Set `tlm.cTime`/`cTimeOld = 5` |
| `ai3.c` `TargetMacArmada` | `cshWar` uninitialized (not reproduced) | Initialize to 0, or make `FPotentMacWarFleet` always store `*pcEquiv` |
| `msg.c` `PszFormatString` | `vrgszUnits[-1]` reads before the array (display text only) | Bounds-check the unit index |
| `tutor.c` `FTutorialEnabledShipBuilder` | Explicit `return TRUE` where the original fell off the end with TRUE in AX | Nothing; drop the `PARITY` marker |

Keep these even after parity no longer matters. They prevent native memory
corruption or match the original toolchain:

- **`ai4.c` `DoCyberAiTurn`:** the `if (iLatest… != -1)` guards on
  `rgRecycleShdef[...] = 0`.
- **`ai3.c` `DoMacintiAiTurn`:** the 16-entry `rgSplitShdef` scratch array for
  turn > 80 splits.
- **`qsort16`** (`win16defines.h`): the Win16 CRT's tie order.
- **x87 rounding casts** to `double`/`float`: deliberate, so don't simplify
  them.

---

## 4. Native port

- **Shims:** every native-port change is marked `NATIVE` and described in
  `docs/WIN16-PARITY.md`. That covers player-message serialization, malformed
  message records, battle heap capacity, static-control colors,
  `WM_CTLCOLOR` dispatch, and two original uninitialized reads.
- **Struct sizes:** `structs.h` layouts and `WriteRt`/`ReadRt` record sizes are
  the file format. 17 structs and `RECT` grow under Win64 (HB, MSGPLR, OBJ,
  PART, PLANET, FLEET, BTN, BTNT, DRAWCIR, RPT, SBAR, SEL, TILE, WN, INI,
  XFER, TUTOR). FLEET and PLANET records pack their fields explicitly, partial
  copies stop before the pointers, and other records keep their Win16 sizes.
  Native pointer or handle sizes never reach a save file.
- **POINT16:** Stars' `POINT` is `POINT16`. Win32 calls use the native `POINT`,
  converted through `PointFrom16`/`PointTo16`. `ChangeScanSel` and
  `DrawBuildSelComp` pass 32-bit `RECT` fields through `POINT16`/`int16_t`
  locals (marked `NATIVE`). Keep this split when adding Win32 calls.
- **Warnings kept** (`-Wall -Wextra -Wno-unused-parameter`, 120):
  - **Unused-but-set (79):** debug-info locals the original also stores to,
    probably for asserts or debug output that was compiled out.
  - **Sign-compare (22):** casts such as `(uint32_t)(dx * dx)` record the
    original's unsigned arithmetic.
  - **Type-limits (14):** enum range checks on unsigned fields.
  - **Unused variables (3):** `ai4.c` `DoCyberAiTurn`'s `shdef`,
    `rgRecycleSBShdef` and `ord`, which the §3 overlay leaves unused. Remove
    them with that revert.
  - **Tautological compare (1):** `aiutil.c` `IroEnsureAi`
    `(iTechCur & 0xf) == 0x1a` is an original dead branch.
  - **Function cast (1):** `ship.c` `TransferStuff` casts
    `FEnumCalcJettison`. Both signatures come from the debug info and are
    ABI-compatible.
- **Original uninitialized reads left as is** (harmless): `ScoreXDlg` and
  `VCRDlg` call `EndDialog(hwnd, i)` with `i` unset, and `CreateChildWindows`
  creates the mine window with an unset `pt` as its size.

---

## 5. Storage widths and boolean conventions

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

---

## 6. After the tag

Bug fixing starts once `2.6jrc3` is tagged. Each fix changes behavior, so the
regression baseline moves with it. Record each one here and, for Win16
behavior, in `docs/WIN16-PARITY.md`.

- **Regression divergences (§2):** fix the Cybertron and Macinti mine-laying
  task unions and initialize `cshWar`.
- **Parity sites (§3):** revert them as listed.
- **Original release bugs:** `docs/known-bugs.md` maps the supplied bug list to
  source and separates located mechanisms from candidates that still need
  reproduction.
- **Tutorial drawing artifact:** an automated tutorial run sometimes draws
  black squares on selected planets. The pre-cleanup build does it too, and it
  hasn't been reproduced by hand. The likely draw sites are the
  selected-planet marker in `scan.c` `DrawScanner` (an 11×11 `SRCAND` mask,
  then an `SRCPAINT` color blit) and the starbase and stargate indicators (a
  5×5 `BLACKNESS` `PatBlt`, then a `PATCOPY` fill). Tracking it down needs
  screenshot capture in the tutorial runner. macOS Wine's GDI capture returns
  blank images, so use another capture method or a Linux/Xvfb run.
