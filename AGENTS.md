# AGENTS.md

Notes for anyone working on this codebase.

## What this is

**CPMToolsQt6** — a Qt 6 front end for CP/M disk images, ported from
**CPMToolsGUI** by neko Java. The CP/M filesystem engine is the portable C
library from [cpmtools](https://github.com/lipro-cpm4l/cpmtools) by Michael
Haardt, reused with only minimal portability fixes.

Current version: **v0.1-preview** (pre-release).

## Hard rules

1. **Do not describe anyone as a "maintainer".** Joseph Kwok
   ([@MustardBun](https://github.com/MustardBun)) is the **author of
   CPMToolsQt6**. There is no commitment to ongoing maintenance, and the
   documentation must not imply one.
2. **Attribution must stay accurate:**
   - CPMToolsGUI — **neko Java** (the program this is ported from)
   - cpmtools engine — **Michael Haardt** <michael@moria.de>,
     primary site <http://www.moria.de/~michael/cpmtools/>
   - Z80-MBC2 disk definitions — **Just4Fun (Fabio Defabis)**
     *(not Marco Maccaferri — that is a common and incorrect attribution)*
   - Qt 6 port — **Joseph Kwok (@MustardBun)**
3. **Licence is GPLv3.** Every file under `qt/`, `tools/` and `scripts/` carries
   an `SPDX-License-Identifier: GPL-3.0-or-later` header. The C engine files keep
   their original upstream headers - do not strip them. Do not add code under an
   incompatible licence.
4. **Keep the engine portable.** `cpmfs.c`, `cpmcp.c`, `cpm_test.c`,
   `mkfs.cpm.c` and `device_posix.c` are shared with the upstream lineage. Prefer
   guarded, behaviour-preserving changes over rewrites.
5. **Do not commit build output.** `build/`, `dist/`, `*.exe`, `*.dll`, `*.qm`
   and release ZIPs are all generated.

## Repository layout

| Path | What it is |
| --- | --- |
| `qt/` | the Qt 6 front end (all UI code lives here) |
| `i18n/` | `.ts` translation catalogues, generated — see below |
| `tools/cpmcli.cpp` | headless harness; drives the engine with no GUI |
| `tools/uishot.cpp` | renders the main window to a PNG off-screen (dev aid) |
| `tools/uitest.cpp` | drives the window off-screen to verify UI behaviour |
| `scripts/` | translation generator and verification scripts |
| `cmake/` | Windows runtime deployment helpers |
| `cpm*.c`, `mkfs.cpm.c`, `device_posix.c` | the portable C engine |
| `diskdefs` | disk geometry definitions, read at run time |

The original Borland C++Builder/VCL sources (`MdiFrame.*`, `MkfsUnit.*`,
`AboutDlg.*`, `CpmtoolsGUI.*`, `*.dfm`) are kept for reference only. They are
**not** part of the Qt build and are excluded from release archives.

## Building

```bash
./build.sh                 # release
./build.sh Debug           # debug
./build.sh Release clean   # wipe build/ first
```

On Windows the toolchain is MSYS2 **UCRT64** (`D:\msys64`). Non-login MSYS2
shells need `PATH=/usr/bin:/ucrt64/bin:$PATH` — `coreutils` and `cmake` live in
different prefixes.

Outputs: `build/CPMToolsQt6.exe` and `build/cpmcli.exe`.

## Gotchas worth knowing

- **`cpm_test.h` is C++-only.** Its prototypes sit in a bare `extern "C" { }`
  with no `__cplusplus` guard, so anything including it must be compiled as C++.
- **The engine reports errors as return values**, not exceptions: a `const char *`
  where `NULL` means success. Check every call.
- **Free `cpm_ls_test()` results with `cpmglobfree()`**, not `free()`. Each name
  is a separate allocation; freeing only the array leaks them.
- **CP/M 8.3 filenames.** A basename longer than 8 characters makes `put` return
  success while writing nothing. This matches the original program's behaviour.
- **`lupdate` is unusable here** — it needs `Qt6Qml.dll` (qt6-declarative, not
  installed). Regenerate catalogues with `python scripts/gen_translations.py`.
- **Resources are declared explicitly** with `qt_add_resources()` per target.
  `AUTORCC` / a `.qrc` file did not reliably register them for secondary targets.
- **PowerShell lookup in CMake** must use the well-known install path
  (`$ENV{SystemRoot}/System32/WindowsPowerShell/v1.0`). CMake invoked from MSYS2
  does not see the Windows directories on `PATH`, and a silent failure there
  produces an executable that dies with `0xC0000135`.
- **A running `CPMToolsQt6.exe` locks the output file**, so relinking fails with
  `Permission denied`. Kill it before rebuilding.
- **CMake caches option values.** Turning an option off requires passing
  `-DOPTION=OFF` explicitly.
- **Any new Qt source must be added to both** the `CPMToolsQt6` and `uishot`
  target source lists in `CMakeLists.txt`.
- **Host folder navigation** lives in `MainWindow`: `setHostFolder()`,
  `applyHostPath()` and `revealHostFile()`. `m_hostFolderPinned` records that the
  user chose a folder, so opening an image must not move the host pane after
  that. Widgets carry object names (`hostPathEdit`, `hostBrowseButton`,
  `hostDirTree`, `hostFileList`, `putAction`) for `uitest` to find.

## Adding a translated string

1. Wrap it in `tr(...)` and, if it is set during construction, register it with
   `setTr(object, "propertyName", tr("..."))` so it re-translates at runtime.
2. Add the exact English string to the relevant context list in
   `scripts/gen_translations.py`.
3. Add the translation to each language dictionary in that file.
4. Run `python scripts/gen_translations.py`, then rebuild.

Language names in the Settings menu are deliberately **not** translated — they
are shown in their own language.

## Verifying changes

```bash
./scripts/engine-smoke.sh          # mkfs / put / ls / get / rm round-trip
```

Expected output ends with `SMOKE_OK`, and the extracted file must be
byte-identical to the original.

To check the UI without a display:

```bash
cmake -S . -B build -G Ninja -DCPM_BUILD_UISHOT=ON
cmake --build build --target uishot uitest
./build/uishot.exe image.dsk out.png 1200 1 ja
./build/uitest.exe image.dsk          # behaviour checks, prints UITEST_OK
```

Arguments for `uishot`: `<image.dsk> <out.png> [width] [fontSize 0-3] [lang]`.
`uitest` exercises host-folder navigation and the Put action, and exits
non-zero on failure. Both need the offscreen platform plugin, which the targets
copy next to the binaries; that plugin is excluded from release packages.

## Release checklist

1. Update the version in `CMakeLists.txt` (`CPM_VERSION_TAG`), `qt/main.cpp`,
   `qt/AboutDialog.cpp` (`kVersion`), `package.sh` and `README.md`.
2. Regenerate translations if UI strings changed.
3. `./build.sh Release clean` — confirm a clean build with no errors.
4. `./scripts/engine-smoke.sh` — confirm `SMOKE_OK`.
5. Launch `build/CPMToolsQt6.exe` with `PATH` stripped to the Windows
   directories to confirm the build is genuinely self-contained.
6. `package_release.ps1` builds the combined release archive (runnable build at
   the root, full source under `src/`). It needs `build/` to have been produced
   first. `./package.sh` is the portable-folder-only variant.
7. Delete superseded archives so only one is ever published.
