#!/usr/bin/env python3
"""Compiles a Max .maxpat (plain JSON) into the binary .amxd chunk format
Ableton Live requires for Max for Live devices.

The .maxpat is the editable source; .amxd is the artifact Live actually
loads. Live refuses to open a bare .maxpat renamed to .amxd - it needs this
binary wrapper: three chunks, each a 4-byte ASCII magic, a 4-byte
little-endian length, and a payload of that length:

    "ampf" + len(4)  + b"aaaa"                  (fixed, unknown-meaning marker)
    "meta" + len(4)  + b"\\x00\\x00\\x00\\x00"    (fixed, 4 zero bytes)
    "ptch" + len(N)  + <the .maxpat file's raw UTF-8 bytes>

Reverse-engineered from a real .amxd exported by Max 8 for this project
(see the "Phase 0" entry in the plan doc for how this was originally found).

Usage:
    python compile_amxd.py "VJ Audio Analyzer.maxpat"

Writes "VJ Audio Analyzer.amxd" next to the source file. Re-run this after
every edit to the .maxpat and copy the result into Live's User Library
(same as the existing workflow described in the plan doc).
"""
import struct
import sys
from pathlib import Path


def chunk(magic: bytes, payload: bytes) -> bytes:
    return magic + struct.pack("<I", len(payload)) + payload


def compile_amxd(maxpat_path: Path) -> Path:
    patcher_json = maxpat_path.read_bytes()

    data = (
        chunk(b"ampf", b"aaaa")
        + chunk(b"meta", b"\x00\x00\x00\x00")
        + chunk(b"ptch", patcher_json)
    )

    amxd_path = maxpat_path.with_suffix(".amxd")
    amxd_path.write_bytes(data)
    return amxd_path


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"usage: python {Path(__file__).name} <path-to.maxpat>")
        sys.exit(1)

    src = Path(sys.argv[1])
    out = compile_amxd(src)
    print(f"wrote {out} ({out.stat().st_size} bytes)")
