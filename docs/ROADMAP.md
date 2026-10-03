# Roadmap to 2.8

The `2.6jrc3` tag is the faithful reconstruction, original bugs included, and
the `2.6j` branch keeps it. `main` is the 2.8 line. Work proceeds in this
order:

1. Native regression baseline and 2.8 repository rules (done).
2. Product versioning from git tags and build numbers (done).
3. The parity reverts below, one commit each.
4. Replacing the `win16defines.h` shims with native Win32 code, one group at
   a time, behavior-neutral.
5. Original bug fixes from [KNOWN-BUGS.md](KNOWN-BUGS.md), and 2.7 features.

Each fix changes behavior, so the native baseline moves with it
(`tests/scaffold/REGRESSION.md`, "Update the native baseline"). Keep each fix
in its own commit, remove it from this list when it lands, and, for Win16
behavior, update [WIN16-PARITY.md](WIN16-PARITY.md) in the same change.

- **Regression divergences** (see
  [REGRESSION.md](../tests/scaffold/REGRESSION.md#known-divergences)): fix
  `cshWar`, then add `oneai6`
  and `smallai6` to the native baseline.
- **Parity sites:** revert the items under "Original bugs emulated" and
  "Known Win16 behavior not reproduced" in
  [WIN16-PARITY.md](WIN16-PARITY.md), as each entry describes:

  | Site                                                 | Parity behavior                                                                                       | Revert to                                                                                                      |
  | ---------------------------------------------------- | ----------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------- |
  | `ai3.c` `TargetMacArmada`                            | `cshWar` uninitialized (not reproduced)                                                               | Initialize to 0, or make `FPotentMacWarFleet` always store `*pcEquiv`                                          |
  | `msg.c` `PszFormatString`                            | `vrgszUnits[-1]` reads before the array (display text only)                                           | Bounds-check the unit index                                                                                    |
  | `tutor.c` `FTutorialEnabledShipBuilder`              | Explicit `return TRUE` where the original fell off the end with TRUE in AX                            | Nothing; drop the `PARITY` marker                                                                              |

  The corruption guards, native record boundaries and toolchain parity
  (`qsort16`, x87 rounding casts) stay.

- **Original release bugs:** [KNOWN-BUGS.md](KNOWN-BUGS.md) maps the supplied
  bug list to source and separates located mechanisms from candidates that
  still need reproduction.
- **Tutorial drawing artifact:** an automated tutorial run sometimes draws
  black squares on selected planets. The pre-cleanup build does it too, and it
  hasn't been reproduced by hand. The likely draw sites are the
  selected-planet marker in `scan.c` `DrawScanner` (an 11×11 `SRCAND` mask,
  then an `SRCPAINT` color blit) and the starbase and stargate indicators (a
  5×5 `BLACKNESS` `PatBlt`, then a `PATCOPY` fill). Tracking it down needs
  screenshot capture in the tutorial runner. macOS Wine's GDI capture returns
  blank images, so use another capture method or a Linux/Xvfb run.
