"""Name original library (CRT) functions by finding the same bytes in the recompiled exe.

  crt-ident.py 0x4328e1 0x43312a ...

Both executables link the same VC9 static CRT, so a library function's body is
byte-identical up to relocated operands. The script takes up to 48 bytes from
each original address (stopping at RET), wildcards absolute operands that point
into the image and rel32 call/jmp targets, searches the recompiled .text, and
prints the recompiled map symbol at each unique hit. Output lines are in
config/verified.csv form when the hit is unique.
"""
import argparse
import os
import re
import sys

import capstone
import pefile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ORIG = os.path.join(ROOT, 'Virtual-City_Win_EN_Full', 'VirtualCity.Unwrapped.exe')
RECOMP = os.path.join(ROOT, 'build', 'vc9', 'bin', 'VirtualCity.Unwrapped.exe')
MAP = os.path.join(ROOT, 'build', 'vc9', 'bin', 'VirtualCity.Unwrapped.map')


def load_map():
    pat = re.compile(r'^\s*[0-9a-fA-F]{4}:[0-9a-fA-F]{8}\s+(\S+)\s+([0-9a-fA-F]{8})\s')
    out = {}
    with open(MAP, 'r', errors='replace') as f:
        for line in f:
            m = pat.match(line)
            if m:
                out.setdefault(int(m.group(2), 16), m.group(1))
    return out


def text_section(pe):
    for s in pe.sections:
        if s.Name.rstrip(b'\0') == b'.text':
            return pe.OPTIONAL_HEADER.ImageBase + s.VirtualAddress, s.get_data()
    raise SystemExit('no .text')


def pattern_for(pe, va, maxlen):
    base = pe.OPTIONAL_HEADER.ImageBase
    code = pe.get_data(va - base, maxlen)
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    parts = []
    lo, hi = base, base + pe.OPTIONAL_HEADER.SizeOfImage
    for ins in md.disasm(code, va):
        raw = bytearray(ins.bytes)
        mask = [True] * len(raw)
        if ins.mnemonic in ('call', 'jmp') and raw[0] in (0xe8, 0xe9):
            for i in range(1, 5):
                mask[i] = False
        else:
            for i in range(len(raw) - 3):
                v = int.from_bytes(raw[i:i + 4], 'little')
                if lo <= v < hi:
                    for j in range(i, i + 4):
                        mask[j] = False
        for b, keep in zip(raw, mask):
            parts.append(re.escape(bytes([b])) if keep else b'.')
        if ins.mnemonic == 'ret' or ins.address + ins.size - va >= maxlen:
            break
    return b''.join(parts)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('addresses', nargs='+')
    ap.add_argument('--len', type=int, default=48)
    args = ap.parse_args()
    orig = pefile.PE(ORIG, fast_load=True)
    rec = pefile.PE(RECOMP, fast_load=True)
    tva, tdata = text_section(rec)
    syms = load_map()
    for a in args.addresses:
        va = int(a, 16)
        pat = pattern_for(orig, va, args.len)
        hits = [tva + m.start() for m in re.finditer(pat, tdata, re.S)]
        names = [syms.get(h, '?%08x' % h) for h in hits]
        if len(hits) == 1 and not names[0].startswith('?%'):
            sym = names[0]
            plain = sym[1:] if sym.startswith('_') else sym
            print('0x%08x|library|%s|%s' % (va, plain.split('@')[0], sym))
        else:
            print('0x%08x  %d hits: %s' % (va, len(hits), ', '.join(names[:6])))


if __name__ == '__main__':
    sys.exit(main())
