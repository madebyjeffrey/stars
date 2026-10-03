# Roadmap to 2.8

The `2.6jrc3` tag is the faithful reconstruction, original bugs included, and
the `2.6j` branch keeps it. `main` is the 2.8 line. Work proceeds in this
order:

1. Native regression baseline and 2.8 repository rules (done).
2. Product versioning from git tags and build numbers (done).
3. Reverting the Win16 parity emulation, one commit each (done).
4. Replacing the `win16defines.h` shims with native Win32 code, one group at
   a time, behavior-neutral (done; what the port still needs is in
   `native.h`/`native.c`).
5. Original bug fixes from [KNOWN-BUGS.md](KNOWN-BUGS.md) (below).
6. 2.7 features from [27I.txt](27I.txt), later.

## 5. Bug fixes

Every fix follows the same steps, in one commit:

1. Add a unit test that reproduces the bug and fails on the current code.
   This is what turns a "located" mechanism into a confirmed bug.
2. Fix it. The test passes.
3. Regenerate the native regression baseline for any scenario that moves
   (`tests/scaffold/REGRESSION.md`, "Update the native baseline") and name
   the moved scenarios and first turn in the commit message.
4. Add a `CHANGELOG.md` entry (mark it **host results** if turn generation
   changes), update the bug's entry in [KNOWN-BUGS.md](KNOWN-BUGS.md), and
   update its row below.

Fixes apply whenever a 2.8 host generates the turn; there is no rules option.
File formats don't change, so 2.6j and 2.8 still share games.

The balance items in [KNOWN-BUGS.md](KNOWN-BUGS.md) (chaff, split fleet
dodge, battle board overload, 0.2% minimum damage, profitable scrapping,
false public scores, mine damage allocation) are reference only and are not
planned.

### 5.1 Unit test harness (done)

Ported the acutest setup from the earlier `stars-decompile` effort; see
[tests/unit/README.md](../tests/unit/README.md). The game sources build once
as the `stars_core` object library, linked into `stars.exe` and into one
acutest executable per `tests/unit/test_<file>.c`. CTest runs the tests
under Wine (`make test-unit`, and the unit-test CI workflow). Acutest runs
each test in its own process, so tests start from fresh globals.

- Function tests call one function with crafted inputs.
- Turn tests use `stars_test.h` to create a tiny game (optionally with AI
  players), load the host file and generate turns.
- The native-port checks moved into `test_native_ports.c`.

Turn-test setup helpers so far: `LpplStarsTestHomeworld`,
`LpflStarsTestAddFleet` and `FStarsTestSaveHost`. Add planet, minefield
and order helpers as fixes need them.

### 5.2 Confirmed in code: test, then fix

| Bug | Mechanism | Test | Status |
| --- | --- | --- | --- |
| Cybertron starbase defenders | `ai4.c` `DoCyberAiTurn`: after `CheckAiShdefStatus(14, 15, …, &iLatestSBDefender, …)` the recycle clear tests and clears `iLatestDestroyer` again | turn | fixed (`test_ai4.c`) |
| North/South minefield immunity | `FIntersectCircleLine`: for a vertical route `yI = ptL1.y` instead of `ptC.y` | function | fixed (`test_utilgen.c`) |
| Space Dock armor overflow | `UpdateShdefCost`: armor slot strength overflows signed 16-bit `dpT` before the RS halving | function | open |
| ISB vs IT gate scanning | `SetVisPFPlanets`, `FGenerateTurn`: `lRadius2 * lVis2` overflows 32 bits before `/ 10000` | function | open |
| Repair after gating | `Merge2Fleets` drops the merged fleet's `fNoHeal` before `HealShips` | turn | open |
| Exploding minefield dodge | `ThingDecay` sets `fBombed` even when the fleet was immune | turn | open |
| Colonization module check | `SatisfyOrders` colonize task doesn't test `cItem > 0` | function | open |
| 32k ships per fleet | `Merge2Fleets` adds into `int16_t rgcsh` without a limit | function | open |
| "Stuck" pursuit | `MoveFleets` marks the pursued fleet `fDone` though it has its own pursuit | turn | open |
| SS pop steal | `SatisfyOrders` transport load sets `fDone` but still loads colonists from an enemy planet | turn | open |
| Mineral upload | `TransferToOthers` destroys cargo the destination can't hold instead of leaving salvage | turn | open |
| Target list overload | `ScannerWndProc`, `ClickInShipOrders` popups stop at 100 entries | UI | open |

### 5.3 Needs reproduction first

Build the reported setup as a turn test. If it misbehaves, find the cause
and move the bug to 5.2. If it doesn't, record that in
[KNOWN-BUGS.md](KNOWN-BUGS.md). The 2.7i release notes in
[27I.txt](27I.txt) list fixes the original developers made after 2.6j, which
can point at causes.

| Bug | Starting point (see KNOWN-BUGS.md) | Status |
| --- | --- | --- |
| East/West speed bump immunity | `FTravelThroughMineFields`, `FIntersectCircleLine` horizontal case | open |
| Cheap starbase (UltraStation variant) | `PROD.pct` against edited designs | open |
| [freepop] hack | `COLDROP` replay in `FRunLogRecord` | open |
| Race file corruption | `WriteRtPlr` name lengths | open |
| Random race | `RaceWizardDlg1` template persistence | open |
| AR starter colonies | starter-base queue entry in `Produce` | open |
| Sappers fail to close to range 2 | `DpFromPtokBrcToBrc` move scoring | open |
| WP0 pop reload ignored | pending drops vs colonize task | open |
| Crash Stars | base-design indices in battle and minefield damage | open |
| Starbase friendly fire | `CplrBattle` attack masks (already changed by the native repair) | open |
| VCR shields and point of view | `RegenShield`, `UpdateShdefCost` use of `idPlayer` | open |

Font problems on non-English Windows and the Netscape attachment corruption
are outside the game code and are not planned.

## Other

- **Tutorial drawing artifact:** an automated tutorial run sometimes drew
  black squares on selected planets, around turn 26 (year 2426). The
  pre-cleanup build did it too, and it hasn't been reproduced by hand. A
  full run after the step 4 cleanup didn't show it; close this after a few
  more clean runs. The likely draw sites are the selected-planet marker in
  `scan.c` `DrawScanner` (an 11×11 `SRCAND` mask, then an `SRCPAINT` color
  blit) and the starbase and stargate indicators (a 5×5 `BLACKNESS`
  `PatBlt`, then a `PATCOPY` fill). Tracking it down needs screenshot
  capture in the tutorial runner. macOS Wine's GDI capture returns blank
  images, so use another capture method or a Linux/Xvfb run.
