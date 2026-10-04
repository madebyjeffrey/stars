# Changelog

Changes from the original Stars! 2.6jrc3 (tag `2.6jrc3`, branch `2.6j`).
Entries marked **host results** change turn generation, so a 2.8 host can
produce different results from a 2.6j host for the same turn.

## 2.8.0 (unreleased)

### Added

- Product version from git tags and build numbers, shown as
  `Version 2.8.0` (or `2.8.0-dev.N+gSHA` between releases) in the About box,
  splash screen and report headers, and stored in a `VERSIONINFO` resource.
  The save-file format version is unchanged (2.83). See
  [docs/VERSIONING.md](docs/VERSIONING.md).

### Fixed

- Macinti AI: deciding whether to build mine layers no longer reads past
  the start of its design list when it has no miner design; a missing design
  counts as zero ships (`DoMacintiAiTurn`). **Host results.**
- Robotoid and Macinti AI: deciding whether an attack fleet joins up with
  others no longer reads a fleet's position as a ship count when the AI has
  no destroyer design (`DoRobotoidAiTurn`, `DoMacintiAiTurn`). **Host
  results.**
- Cybertron AI: mine-laying fleets sent to a random planet now lay mines
  there indefinitely, like the AI's other mine-laying orders. The original
  left the order's countdown as uninitialized stack bytes, so whether the
  fleet laid at all depended on the save path and, after turn 80, it
  usually stopped on arrival (`DoCyberAiTurn`). **Host results.**
- Macinti AI: the same fix for its mine-laying fleets, whose order's
  countdown was uninitialized stack bytes (`DoMacintiAiTurn`). **Host
  results.**
- Macinti AI: an armada too weak to count as a war fleet is now judged by
  its actual strength when choosing whether to attack, retreat or wait. The
  original compared an uninitialized value (`FPotentMacWarFleet`,
  `TargetMacArmada`). **Host results.**
- Message text: a quantity formatted without a preceding mineral no longer
  reads a unit string from before the unit table; it gets no unit
  (`PszFormatString`). Display only.
- Minefields: fleets travelling due north or south now hit minefields along
  their route. The route/field intersection measured a vertical route from
  its start point instead of its closest point to the field
  (`FIntersectCircleLine`). **Host results.**
- Cybertron AI: no longer scraps the ships of its newest starbase-defender
  design once that design passes the recycle age. After checking its
  defender designs it protected the newest destroyer design again instead
  (`DoCyberAiTurn`). **Host results.**
- Ship design: with Regenerating Shields, a large armor slot (such as 24
  Superlatanium on a Space Dock) no longer overflows into a huge armor
  value; it gets half its armor like any other (`UpdateShdefCost`).
  **Host results.**
- Scanning: scanners and stargates with ranges above about 463 ly now see
  cloaked starbases (such as ISB races') within their reduced range. The
  range-times-visibility product overflowed 32 bits, so lightly cloaked
  starbases were often only obscured (`SetVisPFFleets`, `SetVisPFPlanets`,
  `SetVisPFThings`). **Host results.**
- Colonizing: a ship needs an installed colonization or orbital
  construction module. A design slot that once held a module but now holds
  none no longer counts (`SatisfyOrders`). **Host results.**
- Native build: the Battle Plans drop-downs (targets, attack who, tactic)
  saved 0 (None/Disengage, Nobody) whatever was chosen, and the production
  dialog's Required Minerals panel ignored the selected item. Both sent the
  Win16 message numbers for `CB_GETCURSEL`/`LB_GETCURSEL`, which Win32
  controls ignore (`BattlePlansDlg`, `DrawProductionDlg`). Now as in the
  original.
- Native build: the planet and fleet dumps no longer crash. `PszFromLong`
  tested the value behind its optional count pointer instead of the
  pointer, which the dumps pass as NULL; Win16 silently read the start of
  its data segment.
- Native build: the planet dump's Factories column printed 0 and shifted
  the factory count into Def %. The mine and factory counts were passed as
  two 16-bit words each, as the Win16 binary pushed them (`DumpPlanets`).
