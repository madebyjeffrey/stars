# Stars! save CLI

Standalone Go module used by the regression harness. Build from the
repository root with `make save-cli`, then use:

```sh
dist/stars-save save compare LEFT RIGHT
dist/stars-save save compare LEFT RIGHT --show-bytes
dist/stars-save save update game.m1 --ai maid
dist/stars-save save update game.hst --ai maid --player 1
```

Comparison decrypts records and preserves the original comparison rules,
including clock-derived identity exclusions and unused-storage warnings.
Matches and `MATCH with warnings` exit 0; differences and invalid inputs exit 1.
Updates preserve encrypted saves and select a one-based player for host files.

Run tests with `cd tests/savecli && go test ./...`.
