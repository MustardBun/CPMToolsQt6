#!/bin/bash
# Create a CP/M image with a spread of filenames for UI inspection.
cd "/d/Users/Joseph/Downloads/CPMTG20230511/CPMTG(English)/src - vibe" || exit 1
export PATH=/usr/bin:/ucrt64/bin:$PATH

CLI=./build/cpmcli.exe
DEFS=./build/diskdefs
WORK=/tmp/uishot
rm -rf "$WORK" && mkdir -p "$WORK"

"$CLI" "$DEFS" mkfs "$WORK/demo.dsk" ibm-3740 > /dev/null

for n in PROGRAM1.C MAKEFILE.TXT TESTDATA.BIN README.DOC ASSEMBLE.ASM LINKER.REL; do
    printf 'contents of %s\n' "$n" > "$WORK/$n"
    "$CLI" "$DEFS" put "$WORK/demo.dsk" ibm-3740 "$WORK/$n" "$n"
done

echo "image contents:"
"$CLI" "$DEFS" ls "$WORK/demo.dsk" ibm-3740
echo "DEMO=$WORK/demo.dsk"
