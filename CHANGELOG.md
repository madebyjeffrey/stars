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
