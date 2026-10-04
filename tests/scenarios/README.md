# Test scenarios

`stars_scenario` builds small games to open in Stars! by hand, for checks
the unit tests can't make (a popup menu that waits for a click, a dialog
to look at). It links the game code and the unit-test helpers in
`tests/unit/stars_test.h`, like the unit tests.

```sh
make scenario                          # list the scenarios
make scenario SCENARIO=popup-overload  # build dist/scenarios/popup-overload/
```

Each scenario writes `game.xy`, `game.hst` and a turn file per player
(`game.m1`, ...) to `dist/scenarios/<name>/`. Open `game.m1` in Stars! to
play the turn as player 1, or generate turns from `game.hst` as host.

| Scenario | What it is for |
| --- | --- |
| `new-game` | A new one-player tiny universe, unchanged. |
| `popup-overload` | 150 one-ship fleets at the homeworld. Right-click the homeworld: the popup lists all of them, and one past the hundredth can be chosen. Right-click a fleet's waypoint target in its orders: the others are listed. |

## Adding a scenario

Write a builder in `scenarios.c` and add it to `rgscen`:

1. `FStarsTestNewGame(szDir, seed, rgszAi, cAi)` creates the game; AI
   players are `"#<race> <skill>"` definition lines.
2. `FStarsTestLoadHost()` loads it. Change the loaded game directly or
   with the helpers (`LpplStarsTestHomeworld`, `LpflStarsTestAddFleet`).
3. `FStarsTestSaveGame()` writes every player's turn file and the host
   file, as turn generation does.

A builder that needs turns to pass can save the host
(`FStarsTestSaveHost`) and call `FStarsTestGenerate` first. Setup that a
scenario and a unit test share belongs in `stars_test.c`.
