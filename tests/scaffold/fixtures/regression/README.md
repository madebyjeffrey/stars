# Regression fixtures

The `.def` files configure the fixed-seed scenarios. `original/` contains
checkpoints of the original Win16 game, run under DOSBox, for `noai`,
`oneai1`, `oneai2`, `oneai3`, `oneai4`, and `smallai4`: the frozen record of
the original game's behavior. The DOSBox bundle that made them is no longer
kept, so they can't be regenerated.
`native/` contains the native build's checkpoints, the baseline CI compares
against. Its `run.json` lists its scenarios and the hash of the executable
that produced each one. `regression.py export` writes it from a completed run.

Each scenario has checkpoints at turns 0, 1, 10, 25, 50, 80, 100, and 150.
The save bytes are copied unchanged. Each `checkpoint.json` retains the turn,
year, exit code, and SHA-256 hashes; local launch commands are omitted.
`run.json` retains the seed, race and definition hashes, and executable
hashes for provenance and comparison. The full definition-hash map
is retained because the harness checks that map against the native run.
Executables, logs and live saves are not included.

`oneai5`, `oneai6`, and `smallai6` are excluded from `original/`. Their
results depend on original uninitialized reads. 2.8 fixes those reads, so
`native/` covers all nine scenarios.

`Stars.ini` is a minimal unattended test registration for native CLI runs.
It contains no local machine settings. Command-line creation and generation
exit before the interactive machine-registration check.

Regenerate `native/` only in a commit that changes game behavior on purpose
(`make regression`, then `make regression-export`). Keep the
existing launch boundaries when generating native checkpoints. See
[`REGRESSION.md`](../../REGRESSION.md) for running and comparing them.
