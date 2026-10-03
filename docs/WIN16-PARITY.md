# Win16 Parity Repairs

The `2.6j` branch reproduces behavior of the original Stars! 2.6jrc3 that
depends on Win16 memory layout, uninitialized stack bytes, or the original
compiler and runtime, so the fixed-seed regression
(`tests/scaffold/REGRESSION.md`) could show native and original matching
turn for turn. On `main` those emulated bugs are fixed (see
`CHANGELOG.md`); this document lists what remains: guards against native
corruption, native record and message boundaries, and toolchain behavior
the game's results still depend on.

Change the `.c`/`.h` files directly when modifying one of these, and update
this document in the same change with the affected site, original behavior,
and new behavior. Build and run the regression comparisons after each change.

## Corruption guards (keep)

These stop native memory corruption where the original corrupted dead
storage harmlessly. They don't change game behavior and should stay.

- **Cybertron recycle writes for absent designs** (`ai4.c` DoCyberAiTurn):
  `rgRecycleShdef[iLatestDestroyer|iLatestCargo] = 0` with index -1
  overwrote a byte of a dead `lppl` pointer in Win16. Native skips the store
  when the index is -1.
- **Macinti late-game splits** (`ai3.c` DoMacintiAiTurn): turn>80 code
  copied from the 16-entry ship logic marked entries 10–15 of the 10-entry
  `rgRecycleSBShdef`, writing into `l` and `fTonsOfMinerals`. SplitOutShdefs
  read all 16. Native uses a 16-entry `rgSplitShdef` scratch array. The stale
  bytes the original read between calls are not reproduced.
- **Starbase attack mask index** (`battle.c` CplrBattle, marked `NATIVE`):
  in the starbase `iplrAttackEveryone` and `default` (attack one player) cases,
  the original wrote `rggrfAttack[iplrCur]` (BP-0x4c, battle.c:902
  and 917) before `iplrCur` was assigned. The slot held stale stack data, so
  the original wrote the mask to an arbitrary entry, or past the 16-entry
  array, and left the starbase's own entry unset. Native indexes with
  `iplrStarbase`, which the surrounding code evidently intended. This can
  change battle outcomes from the original's. The regression results are
  unchanged. **Revert to:** `rggrfAttack[iplrCur]` at both sites
  (undefined behavior in native).
- **Give-order label width** (`ship.c` DrawShipWayPtOrders, `grTaskGive`,
  marked `NATIVE`): the original measured the "To" label with
  `GetTextExtent(hdc, psz, cch)` while `psz` was still unset (BP-0x32,
  ship.c:377), even though the text is built in and drawn from `szT`. Native
  measures `szT`. Display only. **Revert to:** `psz` (a wild pointer read in
  native).

## Native record and message boundaries (keep)

- **Player-message links** (`log.c` FWriteLogFile/FLoadLogFile and `msg.c`
  WritePlayerMessages/ReadPlayerMessages, marked `NATIVE`): the original
  wrote the four-byte lpmsgplrNext far pointer followed by four int16_t
  fields and abs(cLen) text bytes. Readers immediately replaced that link.
  Native writes four zero bytes and the same payload, then skips the disk
  link when reading. The old `(uint8_t *)&iPlrFrom - 4` writer exposed native
  pointer bits; the reader needlessly copied those bytes into its link.
  Record size remains abs(cLen)+12, and old nonzero links remain readable.
  **Revert to:** raw MSGPLR serialization only with a genuine four-byte
  pointer layout; never restore native pointer serialization.
- **Malformed player-message records** (`log.c` FLoadLogFile and `msg.c`
  ReadPlayerMessages, marked `NATIVE`): a record shorter than its 12-byte
  header plus abs(cLen) text bytes is skipped. The original copied `cb` bytes
  and later wrote abs(cLen)+12 from the heap, over-reading harmlessly; native
  would compute a negative copy length (cb < 4) or overflow the 1024-byte
  write buffer. Valid records (text is limited to 968 characters) are
  unaffected. **Revert to:** nothing; keep the guard.
- **Battle heap capacity** (`file.c` FLoadGame and `battle.c` FDoCoolBattle,
  marked `NATIVE`): original LOWORD(pointer) was an offset within a Win16
  heap segment, whose HB header occupied 16 bytes. Native finds the owning
  HB and subtracts its base, translating sizeof(HB) back to 16 before
  applying the unchanged 0xffc8 limit. Native address low bits previously
  selected rollover arbitrarily, changing battle record order or risking
  overruns. **Revert to:** LOWORD(pointer) only on the original segmented
  heap; keep the relative-offset calculation for native heaps.
- **Static-control colors** (26 dialog checks across battle.c, create.c,
  msg.c, produce.c, race.c, report.c, research.c, scan.c, ship2.c, stars.c,
  tutor.c and utilgen.c, marked `NATIVE`): original WM_CTLCOLOR carried
  CTLCOLOR_STATIC (6) in HIWORD(lParam). Win32 carries the control HWND in
  lParam and the control type in the message ID; these sites now compare
  message/msg with WM_CTLCOLORSTATIC. **Revert to:** the high-word check only
  when using actual Win16 WM_CTLCOLOR dispatch.
- **WM_CTLCOLOR dispatch** (38 window and dialog procedures across
  battle.c, build.c, create.c, mdi.c, mine.c, msg.c, planet.c, produce.c,
  race.c, report.c, research.c, scan.c, ship2.c, stars.c, tb.c, tutor.c and
  utilgen.c, marked `NATIVE`): each original procedure dispatched every
  message, WM_CTLCOLOR included, through one `switch`. Win32 replaced that
  single message with WM_CTLCOLORMSGBOX through WM_CTLCOLORSTATIC, so the
  switch expression maps that range back to WM_CTLCOLOR
  (`switch (IS_WM_CTLCOLOR(message) ? WM_CTLCOLOR : message)`), keeping
  `case WM_CTLCOLOR:` in its original source position. Every other message
  dispatches unchanged. **Revert to:** `switch (message)` with Win16
  WM_CTLCOLOR dispatch.

## Compiler and runtime behavior matched (keep unless parity is dropped)

These follow the original toolchain rather than a bug, and the game's
results depend on them.

- **qsort tie order:** `qsort` maps to `qsort16`
  (implemented in `win16defines.h`), rebuilt from the Win16 CRT, so
  elements with equal keys end up in the same order. Native libc qsort
  orders ties differently.
- **x87 precision:** floating arithmetic keeps the original's extended
  precision. Casts that round an x87 result to double or float are preserved.

## Regression harness tolerances

`dist/stars-save save compare` treats storage the game never reads as warnings,
not differences: order `fUnused`, task-union words past those the task reads,
`tlm.cTimeOld`, AIHIST freighter slots past `cFreighter`, and the reserved
CYBERINFO byte. See `tests/scaffold/REGRESSION.md`.
