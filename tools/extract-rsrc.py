"""Dump the original exe's resources (cursors, icons) byte for byte into a
binary .res for the linker. rc.exe insists on parsing cursor/icon files, so
the .res is written directly. Game assets stay local: the output goes to
build/, never into the tree.

usage: extract-rsrc.py <original.exe> <out.res>
"""
import os
import struct
import sys

import pefile

SKIP_TYPES = {24}  # RT_MANIFEST: the VC9 linker embeds its own


def res_entry(rtype, rname, lang, data):
    header = struct.pack("<HHHH", 0xFFFF, rtype, 0xFFFF, rname)
    header += struct.pack("<IHHII", 0, 0x1030, lang, 0, 0)
    out = struct.pack("<II", len(data), 8 + len(header)) + header + data
    return out + b"\0" * (-len(out) & 3)


def main():
    exe, out = sys.argv[1], sys.argv[2]
    pe = pefile.PE(exe)
    blob = res_entry(0, 0, 0, b"")  # the empty leading entry marks a 32-bit .res
    for rtype in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        if rtype.id is None or rtype.id in SKIP_TYPES:
            continue
        for rname in rtype.directory.entries:
            if rname.id is None:
                continue
            for rlang in rname.directory.entries:
                entry = rlang.data.struct
                data = pe.get_data(entry.OffsetToData, entry.Size)
                blob += res_entry(rtype.id, rname.id, rlang.id, data)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    if not os.path.exists(out) or open(out, "rb").read() != blob:
        with open(out, "wb") as f:
            f.write(blob)


if __name__ == "__main__":
    main()
