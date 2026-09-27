# Changelog

All notable changes to CPMToolsQt6 are recorded here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [v0.1-preview] — 2026-09-27

First public pre-release, shared with the community for testing.

### Added

- Qt 6 port of CPMToolsGUI, replacing the original Win32/VCL front end.
- Three-pane workflow: disk format list, files inside the image, and files on
  the host computer.
- Portable Windows build: the release folder is self-contained and needs no Qt
  or runtime installation.
- Z80-MBC2 and Z80-MBC2-CPM3 disk geometries in the bundled `diskdefs`
  (`z80mbc2-d0`, `z80mbc2-d1`, `z80mbc2-cpm3`).
- Create new images, with optional boot-block files (IPL/CCP/BDOS/BIOS) and
  boot-image skew options.
- Extract, insert and delete files, with drag and drop in both directions.
- Host folder navigation: type or paste a folder path, or browse for a folder
  or a file. Choosing a file shows its folder with the file selected.
- Interface translations for English, Japanese, Simplified Chinese, Traditional
  Chinese, Italian, French, German and Spanish, selected from
  **Settings → Language** and auto-detected from the system locale.
- Adjustable UI font size: Small, Medium, Large and Extra large.
- Help → About dialog with the full attribution list and the complete GPLv3 text.
- Headless `cpmcli` harness for scripted engine checks.
- `uishot` and `uitest` development tools for off-screen UI rendering and
  behaviour verification.

### Fixed

Portability fixes to the shared engine, guarded so the original C++Builder
project still builds:

- `cpmfs.h` no longer redefines `ssize_t` where the toolchain already provides it.
- `cpmReadSuper()` returned the integer `-1` from a function declared to return
  `const char *`; it now returns the error text, as the rest of the file does.
- Explicit cast for the `const char **` parameter of `creat_cpm()`.

Fixes inherited from the original program:

- Directory listings leaked every filename: only the outer array was freed.
  `cpm_ls_test()` results are now released with `cpmglobfree()`.
- A malformed `diskdefs` caused the application to exit abruptly. The Qt front
  end reports the error inline instead.

### Known issues

- **Boot-block image creation is untested.** The UI and engine path exist, but
  the feature has not been verified end to end. Known engine-level concerns
  remain open:

  - `creat_cpm()` ignores the return value of `cpmReadSuper()`, so an unknown
    format can proceed with uninitialised geometry.
  - The output image is opened with `O_CREAT | O_WRONLY` but not `O_TRUNC`, so
    recreating an image with a smaller layout can leave stale trailing data.
  - A failed `read()` of a boot file is not detected, so a partial read can
    produce a corrupted boot area while still reporting success.
  - With boot-image skew enabled, the bounds check compares only the sector's
    starting offset against the buffer size, so the final sector can overrun.
  - The boot-track buffer is not freed when a boot file cannot be opened.

- CP/M 8.3 filename limits are enforced silently by the engine: a base name
  longer than 8 characters reports success but writes nothing. This matches the
  original program's behaviour.

- The Linux build is provided but untested. Windows is the reference platform.

### Attribution

- CPMToolsGUI — neko Java, the original program this is ported from.
- cpmtools engine — © Michael Haardt (GPL).
- Z80-MBC2 disk definitions — Just4Fun (Fabio Defabis).
- Qt 6 port — Joseph Kwok (@MustardBun), developed with AI assistance.

[v0.1-preview]: https://github.com/MustardBun/CPMToolsQt6/releases/tag/v0.1-preview
