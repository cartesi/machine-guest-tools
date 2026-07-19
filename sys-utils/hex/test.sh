#!/bin/sh

set -eu

tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT INT TERM

for size in 0 1 511 512 513 1023 1024 1025 4097; do
    dd if=/dev/urandom of="$tmpdir/input" bs=1 count="$size" 2>/dev/null

    ./hex --encode <"$tmpdir/input" >"$tmpdir/encoded"
    ./hex --decode <"$tmpdir/encoded" >"$tmpdir/decoded"
    cmp "$tmpdir/input" "$tmpdir/decoded"

    ./hex --encode --no-prefix <"$tmpdir/input" >"$tmpdir/encoded"
    ./hex --decode --no-prefix <"$tmpdir/encoded" >"$tmpdir/decoded"
    cmp "$tmpdir/input" "$tmpdir/decoded"
done

printf '\000\001\376\377' >"$tmpdir/expected"
printf '0X0001FEFF' | ./hex --decode >"$tmpdir/decoded"
cmp "$tmpdir/expected" "$tmpdir/decoded"

if printf '0x0' | ./hex --decode >"$tmpdir/decoded" 2>"$tmpdir/error"; then
    echo "hex accepted odd input" >&2
    exit 1
fi
grep -q 'hex string length must be even' "$tmpdir/error"

if printf '0x0g' | ./hex --decode >"$tmpdir/decoded" 2>"$tmpdir/error"; then
    echo "hex accepted an invalid character" >&2
    exit 1
fi
grep -q 'invalid hex character code 103' "$tmpdir/error"

if printf '0010' | ./hex --decode >"$tmpdir/decoded" 2>"$tmpdir/error"; then
    echo "hex accepted a missing prefix" >&2
    exit 1
fi
grep -q 'hex string must start with 0x' "$tmpdir/error"
