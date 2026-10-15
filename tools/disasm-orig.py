"""Disassemble or dump bytes from the original VirtualCity.Unwrapped.exe.

Supplementary evidence only; Ghidra decompiler output stays primary.

  disasm-orig.py 0x462720            # until the first RET followed by int3 padding
  disasm-orig.py 0x462720 0x1e0      # fixed byte length
  disasm-orig.py --dwords 0x4d63f0 [count]   # vtable / data dump, default 8 dwords
  disasm-orig.py --cstr 0x4d5bd8 [0x4d5be8 ...]  # NUL-terminated literals
  disasm-orig.py --recomp 0x401000           # same views of build/vc9/bin's recompiled exe
"""
import argparse
import os
import struct
import sys

import capstone
import pefile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXE = os.path.join(ROOT, 'Virtual-City_Win_EN_Full', 'VirtualCity.Unwrapped.exe')
RECOMP = os.path.join(ROOT, 'build', 'vc9', 'bin', 'VirtualCity.Unwrapped.exe')
MAX_AUTO = 0x4000


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--dwords', action='store_true', help='dump little-endian dwords instead of code')
    ap.add_argument('--keep-int3', action='store_true', help='do not hide int3 padding')
    ap.add_argument('--cstr', action='store_true', help='print NUL-terminated strings at each address')
    ap.add_argument('--recomp', action='store_true', help='read the recompiled exe instead of the original')
    ap.add_argument('address')
    ap.add_argument('length', nargs='*')
    args = ap.parse_args()

    exe = RECOMP if args.recomp else EXE
    pe = pefile.PE(exe, fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    if args.cstr:
        for a in [args.address] + args.length:
            va = int(a, 16)
            data = pe.get_data(va - base, 0x200)
            print('%#010x  %r' % (va, data.split(b'\x00', 1)[0]))
        return
    args.length = args.length[0] if args.length else None
    va = int(args.address, 16)

    if args.dwords:
        count = int(args.length, 0) if args.length else 8
        words = struct.unpack('<%dI' % count, pe.get_data(va - base, count * 4))
        for i, w in enumerate(words):
            print('%#010x  +%#04x  %#010x' % (va + i * 4, i * 4, w))
        return

    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    if args.length:
        data = pe.get_data(va - base, int(args.length, 16))
        for ins in md.disasm(data, va):
            if ins.mnemonic == 'int3' and not args.keep_int3:
                continue
            print('%#x %s %s' % (ins.address, ins.mnemonic, ins.op_str))
        return

    # Auto length: stop at a RET that is not jumped over, i.e. followed by
    # int3/nop padding or by an address no earlier branch targets.
    data = pe.get_data(va - base, MAX_AUTO)
    targets = set()
    for ins in md.disasm(data, va):
        print('%#x %s %s' % (ins.address, ins.mnemonic, ins.op_str))
        if ins.mnemonic.startswith('j') and ins.op_str.startswith('0x'):
            targets.add(int(ins.op_str, 16))
        if ins.mnemonic in ('ret', 'retn') or (ins.mnemonic == 'jmp' and not ins.op_str.startswith('0x')):
            nxt = ins.address + ins.size
            if not any(t >= nxt for t in targets):
                return
    sys.stderr.write('stopped at %#x bytes without a final RET\n' % MAX_AUTO)


if __name__ == '__main__':
    main()
