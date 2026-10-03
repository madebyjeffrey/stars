# Regression fixtures

The `.def` files configure the fixed-seed scenarios. `original/` contains
existing DOSBox REGTEST checkpoints for `noai`, `oneai1`, `oneai2`, `oneai3`,
`oneai4`, and `smallai4`: the record of the original game's behavior.
`native/` contains the native build's checkpoints, the baseline CI compares
against. Its `run.json` lists its scenarios and the executable hash that
produced it. `regression.py export` writes it from a completed run.

Each scenario has checkpoints at turns 0, 1, 10, 25, 50, 80, 100, and 150.
The save bytes are copied unchanged. Each `checkpoint.json` retains the turn,
year, exit code, and SHA-256 hashes; local launch commands are omitted.
`run.json` retains the seed, race and definition hashes, and original/seeded
executable hashes for provenance and comparison. The full definition-hash map
is retained because the harness checks that map against the native run.
Executables, logs, live saves, and DOSBox configuration are not included.

`oneai5`, `oneai6`, and `smallai6` are excluded from `original/`. Their
results depend on original uninitialized reads, so they are added to
`native/` only once those reads are fixed. Their definitions remain
available for manual investigation with the local starsbox reference.

`Stars.ini` is a minimal unattended test registration for native CLI runs.
Its `GlobalSettings` encodes the tutorial harness's default serial `CV6JVUAX`
with an all-zero machine configuration, produced and round-trip verified by
the game's `FormatSerialAndEnv` and `FSerialAndEnvFromSz` functions. Copy it
into an isolated Wine prefix's `drive_c/windows/Stars.ini` after `wineboot`.
It contains no local machine settings. Command-line creation and generation
exit before the interactive machine-registration check.

Do not regenerate the DOSBox checkpoints for routine CI changes. Regenerate
`native/` only in a commit that changes game behavior on purpose. Keep the
existing launch boundaries when generating native checkpoints. See
[`REGRESSION.md`](../../REGRESSION.md) for running and comparing them.
