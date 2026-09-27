# Contributing to CPMToolsQt6

Thanks for taking an interest. This is a hobby project released as a
pre-release, so please read the notes below before sending changes.

## Please note

**There is no maintenance commitment.** CPMToolsQt6 is published by its author,
Joseph Kwok ([@MustardBun](https://github.com/MustardBun)), without any promise
of ongoing support or a release schedule. Bug reports and pull requests are
welcome, but they may sit for a while, and some may be declined.

The most valuable contribution by far is a **clear bug report** — especially for
the Z80-MBC2 workflows and boot-block image creation, which have had the least
testing.

## Reporting a bug

Open an issue and include:

- What you did, what you expected, and what actually happened
- The disk format you selected, the image size, and how the image was created
- Whether it reproduces from a freshly formatted image
- Your Windows version and whether you used the packaged build or your own
- Screenshots for anything visual

Log/console output helps. The packaged build is compiled with a console attached
so engine diagnostics are visible when launched from a terminal.

## Building

See the **Building** section of [README.md](README.md). In short, on Windows use
MSYS2 UCRT64:

```bash
./build.sh Release clean
```

## Before sending a pull request

1. **Build cleanly.** `./build.sh Release clean` with no errors.
2. **Run the engine smoke test.** `./scripts/engine-smoke.sh` must end with
   `SMOKE_OK` and the round-tripped file must be byte-identical.
3. **Run the UI checks** if you touched the interface:

   ```bash
   cmake -S . -B build -G Ninja -DCPM_BUILD_UISHOT=ON
   cmake --build build --target uitest
   ./build/uitest.exe build/diskdefs
   ```

   It must print `UITEST_OK`.
4. **Keep the engine portable.** `cpmfs.c`, `cpmcp.c`, `cpm_test.c`,
   `mkfs.cpm.c` and `device_posix.c` are shared with the upstream lineage. Prefer
   guarded, behaviour-preserving changes over rewrites, and keep the original
   C++Builder project buildable.

## Attribution and licence

This project is a derived work, so please keep the credits intact:

- **CPMToolsGUI** — **neko Java**, the program this is ported from
- **cpmtools engine** — **Michael Haardt** <michael@moria.de>,
  primary site <http://www.moria.de/~michael/cpmtools/>
- **Z80-MBC2 disk definitions** — **Just4Fun (Fabio Defabis)**. Please leave
  this credit as it is; other names circulating online are incorrect.
- **Qt 6 port** — **Joseph Kwok (@MustardBun)**

Everything here is **GPLv3**. New files go under
`SPDX-License-Identifier: GPL-3.0-or-later`, and the original upstream headers
in the engine files must not be stripped.

Please do not commit generated files — `build/`, `dist/`, `*.exe`, `*.dll`,
`*.qm` and the release ZIPs are all build output.

## Translating

The string catalogue lives in `scripts/gen_translations.py`, which is the single
source of truth. `lupdate` is deliberately not used (it needs the
qt6-declarative tooling). To add or correct a translation:

1. Add the exact English source string to the right context list.
2. Add the translation to each language dictionary.
3. Run `python scripts/gen_translations.py`, then rebuild.

Language names in the Settings menu are intentionally left untranslated — they
are shown in their own language.

## Licence

By contributing you agree that your work is licensed under the GNU General
Public License v3.0 or later.
