# CPMToolsQt6

A modern, cross-platform Qt 6 front end for CP/M disk images — browse, create
and edit CP/M volumes, including Z80-MBC2 CP/M&nbsp;3 disks.

CPMToolsQt6 is a port of **CPMToolsGUI** by **neko Java**, rebuilt on Qt&nbsp;6
and reusing the portable C engine from
[cpmtools](http://www.moria.de/~michael/cpmtools/) by Michael Haardt.

- **Author of CPMToolsQt6:** Joseph Kwok ([@MustardBun](https://github.com/MustardBun))
- **Version:** v0.1-preview *(pre-release preview)*
- **Licence:** GNU General Public License v3.0 (GPLv3) — see [`LICENSE`](LICENSE)
- **Changes:** see [`CHANGELOG.md`](CHANGELOG.md)
- **Downloads:** [Releases](https://github.com/MustardBun/CPMToolsQt6/releases)

> **Pre-release.** This is an early preview build shared for testing. Expect
> rough edges and please report anything that misbehaves.

### Known issues in this build

- **Boot-block image creation is untested.** Supplying up to four boot files
  when creating an image is wired up, but the path has not been verified end to
  end. See [CHANGELOG.md](CHANGELOG.md) for the specific engine-level concerns.
- **8.3 filenames are enforced silently.** A base name longer than 8 characters
  reports success but writes nothing. This matches the original program.
- **The Linux build is untested.** Windows is the reference platform.
- Any string without a translation falls back to English.

---

## Overview

Three panes: the list of supported disk formats, the files inside the image, and
the files on your computer. Move files in either direction by button, menu, or
drag and drop.

### Features

- **Z80-MBC2 / Z80-MBC2-CPM3 disk support** — the bundled `diskdefs` includes
  the geometries for the Z80-MBC2 single-board computers
  (`z80mbc2-d0`, `z80mbc2-d1`, `z80mbc2-cpm3`) alongside the classic formats.
- **73 disk formats** out of the box, read from a `diskdefs` file.
- **Browse images** — list directory entries with a CP/M wildcard filter
  (`*.*`, `*.com`, `*.asm`, …).
- **Extract and insert files** between the image and the host filesystem,
  with drag and drop in both directions.
- **Reach any folder easily** — type or paste a folder path straight into the
  host pane, or use **Browse...** to open either a folder or a single file.
  Picking a file shows its folder with that file already selected. The folder
  tree is still there for quick navigation.
- **Delete** entries from an image.
- **Create new images**, including optional boot-block images
  (IPL/CCP/BDOS/BIOS) and boot-image skew options.
- **Drag and drop** — drag files out of an image to Explorer, or drop files onto
  the image pane to insert them.
- **Internationalised** — English, Japanese, Simplified Chinese, Traditional
  Chinese, Italian, French, German and Spanish.
- **Adjustable UI font size** — Small / Medium / Large / **Extra large**.
- **Portable** — the Windows build is a self-contained folder; no Qt
  installation is required on the target machine.

---

## Getting CPMToolsQt6

### Windows (prebuilt)

Download `CPMToolsQt6-v0.1-preview.zip`. The executable and everything it needs
are at the top level — unzip, then run `CPMToolsQt6.exe`. No installation is
required: Qt, the MinGW runtime and `diskdefs` are all bundled. Requires 64-bit
Windows 10 or later.

The same archive carries the complete source under `src/` if you would rather
build it yourself.

### Running from a build tree

`build/` is self-contained and can be run directly:

```powershell
.\build\CPMToolsQt6.exe
```

A disk image can be passed on the command line:

```powershell
.\build\CPMToolsQt6.exe mydisk.dsk
```

---

## Building

The engine is portable C; the front end needs Qt 6 (Widgets) and CMake 3.21+.
Ninja is recommended but optional.

### Windows (MSYS2 / MinGW-w64)

MSYS2 with the **UCRT64** toolchain is the smoothest option.

```bash
pacman -S --needed \
    mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-ninja \
    mingw-w64-ucrt-x86_64-qt6-base \
    mingw-w64-ucrt-x86_64-qt6-tools
```

Then, from the MSYS2 shell (or via `D:\msys64\usr\bin\bash.exe`):

```bash
./build.sh                 # release build
./build.sh Debug           # debug build
./build.sh Release clean   # wipe build/ first
```

Or directly:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The build runs `windeployqt` and resolves the remaining MinGW runtime
dependencies automatically, leaving `build/` self-contained.

### Linux (Debian / Ubuntu)

```bash
sudo apt update
sudo apt install -y \
    build-essential cmake ninja-build \
    qt6-base-dev qt6-tools-dev qt6-tools-dev-tools qt6-base-dev-tools \
    libgl1-mesa-dev

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/CPMToolsQt6
```

`diskdefs` is read from beside the executable:

```bash
cp diskdefs build/
```

### Linux (Fedora / Arch)

```bash
# Fedora
sudo dnf install gcc-c++ cmake ninja-build qt6-qtbase-devel qt6-qttools-devel

# Arch
sudo pacman -S base-devel cmake ninja qt6-base qt6-tools
```

> Windows is the reference platform and the only one exercised end-to-end. The
> engine and front end are written to be portable and `CONFIG.H` is platform
> conditional, but the Linux build has had little testing. Feedback welcome.

### Build options

| Option | Default | Meaning |
| --- | --- | --- |
| `CPM_BUILD_GUI` | `ON` | build the Qt GUI |
| `CPM_BUILD_CLI` | `ON` | build `cpmcli`, the headless harness |
| `CPM_PORTABLE` | `ON` | bundle the Qt + MinGW runtime into the build folder |
| `CPM_STATIC_RUNTIME` | `ON` | statically link the GCC runtime into `cpmcli` |
| `CPM_CONSOLE` | `ON` | attach a console so engine diagnostics are visible |
| `CPM_BUILD_UISHOT` | `OFF` | build `uishot`, the off-screen screenshot tool |

---

## Packaging a release

The release archive contains **both** the runnable Windows build and the full
source, so one download serves everyone:

```
CPMToolsQt6-v0.1-preview/
    CPMToolsQt6.exe        <- ready to run, nothing to install
    cpmcli.exe             <- headless engine harness
    diskdefs               <- disk geometry definitions
    Qt6*.dll  platforms/   <- bundled Qt and MinGW runtime
    README.md  LICENSE
    src/                   <- complete source tree
```

Build first, then package:

```bash
./build.sh Release clean
powershell -ExecutionPolicy Bypass -File package_release.ps1
# -> CPMToolsQt6-v0.1-preview.zip
```

Pass `-Version` to override the tag:

```powershell
powershell -ExecutionPolicy Bypass -File package_release.ps1 -Version v0.1.1-preview
```

To produce only the portable Windows folder (no source, no ZIP):

```bash
cmake --build build --target portable
# -> build/dist/CPMToolsQt6-v0.1-preview-win64/
```

---

## Localisation

The interface ships in eight languages. On first run the language is detected
from the system locale and falls back to English when there is no match.

| Code | Language |
| --- | --- |
| `en` | English |
| `ja` | 日本語 |
| `zh_CN` | 简体中文 |
| `zh_TW` | 繁體中文 |
| `it` | Italiano |
| `fr` | Français |
| `de` | Deutsch |
| `es` | Español |

Switch language at runtime under **Settings → Language**, including a
*System default* entry. The choice is remembered between runs.

The catalogues are compiled with `lrelease` and embedded in the executable, so
no `.qm` files ship alongside the binary.

### Updating the translations

The string catalogue and its translations live in `scripts/gen_translations.py`:

```bash
python scripts/gen_translations.py    # rewrites i18n/*.ts
cmake --build build                   # lrelease compiles and embeds them
```

`lupdate` is deliberately not part of the build (it needs the
qt6-declarative tooling); the generator script is the single source of truth.

---

## Project layout

```
cpmfs.c  cpmcp.c  cpm_test.c  mkfs.cpm.c  device_posix.c   portable C engine
cpm_test.h  cpmfs.h  mkfs.cpm.h  device.h  CONFIG.H        engine headers
diskdefs                                                   format definitions
LICENSE                                                    GPLv3
CHANGELOG.md                                               release history
CONTRIBUTING.md                                            how to report and contribute

qt/                        Qt 6 front end
  main.cpp                 entry point
  MainWindow.*             main browser window
  MkfsDialog.*             "new image" dialog
  AboutDialog.*            about box, credits and the GPLv3 text
  CpmBackend.*             wrapper around the engine entry points
  Translator.*             language detection and switching
  AppSettings.*            persisted UI preferences
  FileDropListWidget.h     list that accepts drops and starts drags
  FileDropLineEdit.h       line edit that accepts a dropped file

i18n/                      .ts catalogues for the eight languages
cmake/                     deployment helpers
tools/cpmcli.cpp           headless engine harness
tools/uishot.cpp           off-screen screenshot tool (dev aid)
scripts/                   generator and verification scripts
```

---

## Verifying a build

```bash
./scripts/engine-smoke.sh          # formats, inserts, extracts, compares, deletes
```

The harness `cpmcli` drives the same engine entry points as the GUI:

```bash
./build/cpmcli.exe ./build/diskdefs fmt
./build/cpmcli.exe ./build/diskdefs mkfs test.dsk ibm-3740
./build/cpmcli.exe ./build/diskdefs ls test.dsk ibm-3740
```

---

## Credits and attribution

This program is a derived work. With thanks to:

- **CPMToolsGUI** by **neko Java** — the original CP/M tools GUI that
  CPMToolsQt6 is ported from, including the `diskdefs` format definitions and
  the MITS Altair 88-DISK support.
- **cpmtools engine** — written by **Michael Haardt** <michael@moria.de>.
  The portable C library that reads and writes the CP/M filesystem:
  `cpmfs.c`, `cpmcp.c`, `mkfs.cpm.c`, `device_posix.c`.
  *Primary site:* <http://www.moria.de/~michael/cpmtools/> — licensed under the GPL.
- **Z80-MBC2 disk definitions** — **Just4Fun (Fabio Defabis)**. Disk geometries
  for the Z80-MBC2 and Z80-MBC2-CPM3 single-board computers.
- **Qt 6 port** — © 2026 Joseph Kwok (@MustardBun). The Qt 6 user interface,
  build system, packaging and localisation.

CP/M is a trademark of Digital Research / DRDOS, Inc. This project is not
affiliated with or endorsed by any of the parties above.

---

## Licence

Copyright (C) 2026 Joseph Kwok (@MustardBun)

This program is free software: you can redistribute it and/or modify it under
the terms of the **GNU General Public License** as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>. The full text is also
in [`LICENSE`](LICENSE) and in the program's **Help → About** dialog.
