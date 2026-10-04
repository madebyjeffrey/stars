# Versioning

Stars! has two version numbers, and only one of them changes on `main`.

## Product version

The version shown in the About box, on the splash screen and in report
headers (`SzVersion` in `stars.c`), and stored in the executable's
`VERSIONINFO` resource. `cmake/version.cmake` generates it from git into
`<build>/generated/version.h` on every build:

| Checkout | Version | Build |
| --- | --- | --- |
| Clean checkout of tag `v2.8.0` | `2.8.0` | 0 |
| 12 commits after `v2.8.0` | `2.8.1-dev.12+g1a2b3c4` | 12 |
| Same, with uncommitted changes | `2.8.1-dev.12+g1a2b3c4.dirty` | 12 |
| Before the first `v` tag | `2.8.0-dev.N+g…`, N counted from `2.6jrc3` | N |

A development build is numbered toward the next version: the patch after the
last `vX.Y.Z` tag, or `project(stars VERSION …)` in `CMakeLists.txt` if that
is higher. To start a new minor version (2.9.0), raise the `project()`
version; patch releases need no version commit. `VERSIONINFO` carries
`MAJOR,MINOR,PATCH,BUILD`.

Source archives without git history build as `X.Y.Z-dev.0+gunknown`; set
`STARS_VERSION=2.8.0` in the environment to stamp them.

## Releasing

```sh
git tag v2.8.0
git push origin v2.8.0
```

The release workflow builds the tag, checks that it produced exactly
`2.8.0`, and publishes "Stars! 2.8.0" with `stars.exe` and `stars!.hlp`.
Every push to `main` updates the rolling `latest` prerelease, whose title
shows the build's version. Other tags (such as `2.6jrc3` on the `2.6j`
branch) still publish releases named after the tag.

## File format version

Saves, turn files and race files carry their own version in `RTBOF`
(`verMajor`/`verMinor`/`verInc`). The original wrote 2.83 (`save.c`) and
refused files from 2.84 or later (`file.c`, `mdi.c`, `save.c`). 2.8 writes
2.84 and reads 2.49 through 2.84: it picks up games, turns and races from
2.6j/2.7, while 2.6j/2.7 refuse files written by 2.8 instead of playing on
with different rules. The records themselves are unchanged. It is not tied
to the product version; change it only together with a real file-format
change, and expect older versions to reject the files.
