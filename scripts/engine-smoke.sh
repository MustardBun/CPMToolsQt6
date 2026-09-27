#!/bin/bash
# End-to-end check of the CP/M engine through the cpmcli harness.
# Run from anywhere; it locates the project relative to this script.
#
# The pass/fail logic uses bash builtins only, so this runs in a bare MSYS2/MinGW
# environment without coreutils or diffutils - which is what the GitHub Actions
# runners provide.
set -e

# Resolve the project root from this script's location using bash alone, so
# dirname is not required.
script_path=${BASH_SOURCE[0]}
case $script_path in
    */*) script_dir=${script_path%/*} ;;
    *)   script_dir=. ;;
esac
ROOT="$(cd "$script_dir/.." && pwd)"
cd "$ROOT" || exit 1

export PATH=/usr/bin:/ucrt64/bin:$PATH

CLI=./build/cpmcli.exe
DEFS=./build/diskdefs
FORMAT="${1:-ibm-3740}"

# A writable scratch directory. mkdir may be missing in a bare environment, so
# fall back to the current directory and no cleanup.
WORK="${TMPDIR:-/tmp}/cpmtest"
if command -v mkdir >/dev/null 2>&1; then
    WORK_DIR_OK=1
    if command -v rm >/dev/null 2>&1; then rm -rf "$WORK"; fi
    mkdir -p "$WORK" 2>/dev/null || WORK_DIR_OK=0
else
    WORK="."
    WORK_DIR_OK=1
fi
if [ "$WORK_DIR_OK" -ne 1 ]; then
    work="./cpmtest"
    mkdir -p "$work" >/dev/null 2>&1 || true
    [ -d "$work" ] && WORK="$work"
fi

if [ ! -f "$CLI" ]; then
    echo "error: $CLI not built yet - run ./build.sh first" >&2
    exit 1
fi

if [ ! -f "$DEFS" ]; then
    echo "error: $DEFS not found - is diskdefs in the project root?" >&2
    exit 1
fi

fail=0
step() { echo; echo "== $*"; }
expect_ok() { if [ "$1" -ne 0 ]; then echo "FAIL: $2 (exit $1)" >&2; fail=1; else echo "ok: $2"; fi; }

step "list formats (first 5)"
# bash read loop rather than head, which may not be installed.
"$CLI" "$DEFS" fmt 2>&1 | {
    n=0
    while IFS= read -r line && [ "$n" -lt 5 ]; do
        echo "$line"
        n=$((n + 1))
    done
}

step "create image with format '$FORMAT'"
"$CLI" "$DEFS" mkfs "$WORK/test.dsk" "$FORMAT"
expect_ok $? "mkfs"

if [ -f "$WORK/test.dsk" ]; then
    echo "ok: image created"
else
    echo "FAIL: image file was not created" >&2
    fail=1
fi

step "put a host file into the image"
printf 'Hello from CP/M\r\n' > "$WORK/HELLO.TXT"
"$CLI" "$DEFS" put "$WORK/test.dsk" "$FORMAT" "$WORK/HELLO.TXT" HELLO.TXT
expect_ok $? "put"

step "list the image"
"$CLI" "$DEFS" ls "$WORK/test.dsk" "$FORMAT"

step "extract it again"
"$CLI" "$DEFS" get "$WORK/test.dsk" "$FORMAT" HELLO.TXT "$WORK/OUT.TXT"
expect_ok $? "get"

# Byte comparison with bash alone; $(<file) is a builtin, so cmp is not needed.
if [ -f "$WORK/OUT.TXT" ]; then
    original=$(<"$WORK/HELLO.TXT")
    extracted=$(<"$WORK/OUT.TXT")
    if [ "$original" = "$extracted" ]; then
        echo "ok: round-trip is byte identical (${#extracted} bytes)"
    else
        echo "FAIL: round-trip mismatch (${#original} vs ${#extracted} bytes)" >&2
        fail=1
    fi
else
    echo "FAIL: nothing was extracted" >&2
    fail=1
fi

step "delete it"
"$CLI" "$DEFS" rm "$WORK/test.dsk" "$FORMAT" HELLO.TXT
expect_ok $? "rm"

# The deleted file must no longer appear in the listing.
listing=$("$CLI" "$DEFS" ls "$WORK/test.dsk" "$FORMAT" 2>&1)
if [[ "$listing" == *"hello"* || "$listing" == *"HELLO"* ]]; then
    echo "FAIL: the file is still listed after deletion" >&2
    fail=1
else
    echo "ok: file no longer listed"
fi

if command -v rm >/dev/null 2>&1; then rm -rf "$WORK"; fi

echo
if [ "$fail" -eq 0 ]; then echo "SMOKE_OK"; else echo "SMOKE_FAILED"; exit 1; fi
