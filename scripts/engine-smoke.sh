#!/bin/bash
# End-to-end check of the CP/M engine through the cpmcli harness.
# Run from anywhere; it locates the project relative to this script.
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT" || exit 1
export PATH=/usr/bin:/ucrt64/bin:$PATH

CLI=./build/cpmcli.exe
DEFS=./build/diskdefs
WORK="${TMPDIR:-/tmp}/cpmtest"
FORMAT="${1:-ibm-3740}"

if [ ! -x "$CLI" ]; then
    echo "error: $CLI not built yet - run ./build.sh first" >&2
    exit 1
fi

rm -rf "$WORK" && mkdir -p "$WORK"

fail=0
step() { echo; echo "== $*"; }
expect_ok() { if [ "$1" -ne 0 ]; then echo "FAIL: $2 (exit $1)" >&2; fail=1; else echo "ok: $2"; fi; }

step "list formats (first 5)"
"$CLI" "$DEFS" fmt 2>&1 | head -5

step "create image with format '$FORMAT'"
"$CLI" "$DEFS" mkfs "$WORK/test.dsk" "$FORMAT"
expect_ok $? "mkfs"
ls -la "$WORK/test.dsk"

step "put a host file into the image"
printf 'Hello from CP/M\r\n' > "$WORK/HELLO.TXT"
"$CLI" "$DEFS" put "$WORK/test.dsk" "$FORMAT" "$WORK/HELLO.TXT" HELLO.TXT
expect_ok $? "put"

step "list the image"
"$CLI" "$DEFS" ls "$WORK/test.dsk" "$FORMAT"

step "extract it again"
"$CLI" "$DEFS" get "$WORK/test.dsk" "$FORMAT" HELLO.TXT "$WORK/OUT.TXT"
expect_ok $? "get"
if cmp -s "$WORK/HELLO.TXT" "$WORK/OUT.TXT"; then
    echo "ok: round-trip is byte identical"
else
    echo "FAIL: round-trip mismatch" >&2
    fail=1
fi

step "delete it"
"$CLI" "$DEFS" rm "$WORK/test.dsk" "$FORMAT" HELLO.TXT
expect_ok $? "rm"
"$CLI" "$DEFS" ls "$WORK/test.dsk" "$FORMAT"

echo
if [ "$fail" -eq 0 ]; then echo "SMOKE_OK"; else echo "SMOKE_FAILED"; exit 1; fi
