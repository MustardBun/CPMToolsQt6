# CPMToolsQt6 v0.1-preview

**First public pre-release.** A Qt 6 front end for CP/M disk images, ported from
CPMToolsGUI by neko Java. Please test it and report anything that misbehaves.

---

## Download

**`CPMToolsQt6-v0.1-preview.zip`** (≈33 MB)

Unzip anywhere and run **`CPMToolsQt6.exe`**. Nothing needs to be installed —
Qt, the MinGW runtime and `diskdefs` are all inside the folder.

*Requires 64-bit Windows 10 or later.*

The same archive contains the complete source under `src/` if you would rather
build it yourself.

---

## What it does

Browse, create and edit CP/M disk images, including Z80-MBC2 CP/M 3 volumes.

- **73 disk formats** out of the box, read from `diskdefs`
- **Z80-MBC2 support** — `z80mbc2-d0`, `z80mbc2-d1` and `z80mbc2-cpm3`
- **Extract, insert and delete** files, with drag and drop both ways
- **Create new images**, with optional boot-block images and boot-image skew
- **Easy folder navigation** — type or paste a folder path, or browse for a
  folder or a single file
- **Eight interface languages** — English, 日本語, 简体中文, 繁體中文, Italiano,
  Français, Deutsch, Español
- **Adjustable font size** — Small / Medium / Large / Extra large
- **Portable** — no installation, no registry footprint beyond user preferences

---

## Using it

1. **Select...** an image file (or drop one onto the path field).
2. Pick the **Format** — the list comes from `diskdefs`.
3. The middle pane lists the files inside the image.
4. **Put into image** copies the files selected on the right in;
   **Get from image** copies the selected image files out.
5. **File → New image...** formats a fresh volume.

A disk image can also be passed on the command line:

```
CPMToolsQt6.exe mydisk.dsk
```

### Keyboard and convenience

- **Ctrl+O** open image · **Ctrl+N** new image · **Del** delete entry
- **Tab** to the folder path field, paste a path, press **Enter**
- **Drag files out** of the image pane to Explorer to copy them out
- **Drop files** onto the image pane to add them

---

## Known issues in this build

This is a **pre-release**, so please set expectations accordingly.

- **Boot-block image creation is untested.** You can supply up to four boot
  files when creating an image, but that path has not been verified end to end.
  Known engine-level concerns are listed in `CHANGELOG.md`. If you try it,
  please report what happens — especially with real Z80-MBC2 IPL/CCP/BDOS/BIOS
  files.
- **8.3 filenames are enforced silently.** A base name longer than 8 characters
  reports success but writes nothing. This matches the original program.
- **The Linux build is untested.** Windows is the reference platform.
- English is the fallback for any string not yet translated.

---

## Reporting problems

Please include:

- What you did, what you expected, what happened
- The disk format used, and the image size if relevant
- Whether it reproduces from a freshly formatted image
- Your Windows version

Screenshots help a lot for UI issues.

---

## Credits

CPMToolsQt6 is a derived work:

- **CPMToolsGUI** — **neko Java**, the original program this is ported from,
  including the `diskdefs` format definitions and the MITS Altair 88-DISK support
- **cpmtools engine** — written by **Michael Haardt** <michael@moria.de>, the
  portable C library that reads and writes the CP/M filesystem (GPL).
  Primary site: <http://www.moria.de/~michael/cpmtools/>
- **Z80-MBC2 disk definitions** — **Just4Fun (Fabio Defabis)**
- **Qt 6 port** — © 2026 **Joseph Kwok (@MustardBun)**

CP/M is a trademark of Digital Research / DRDOS, Inc. This project is not
affiliated with or endorsed by any of the parties above.

---

## Licence

**GNU General Public License v3.0.** See `LICENSE` in the archive, or the
**Help → About** dialog, which shows the full text. Source is included with the
binary, as the licence requires.
