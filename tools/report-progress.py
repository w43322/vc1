"""Summarize decomp progress from the last tools/compare-reccmp.ps1 run.

Reads build/vc9/reports/VC1.json (asmcmp) and VC1-roadmap.csv (sizes). Byte
coverage uses the roadmap row size against the original .text size, so it is
an estimate, not overall game completion.
"""
import csv
import json
import os

import pefile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REPORTS = os.path.join(ROOT, 'build', 'vc9', 'reports')
EXE = os.path.join(ROOT, 'Virtual-City_Win_EN_Full', 'VirtualCity.Unwrapped.exe')


def main():
    data = json.load(open(os.path.join(REPORTS, 'VC1.json'), encoding='utf-8'))['data']
    sizes = {}
    with open(os.path.join(REPORTS, 'VC1-roadmap.csv'), encoding='utf-8') as f:
        for row in csv.DictReader(f):
            if row['row_type'] == 'fun' and row['orig_addr']:
                sizes[int(row['orig_addr'], 16)] = int(row['size'], 16)

    pe = pefile.PE(EXE, fast_load=True)
    text = next(s for s in pe.sections if s.Name.rstrip(b'\0') == b'.text')
    text_size = text.Misc_VirtualSize

    funcs = [e for e in data if e['type'] == 1]
    vtables = [e for e in data if e['type'] == 5]
    stubs = [e for e in funcs if e.get('stub')]
    library = [e for e in funcs if e.get('library')]
    real = [e for e in funcs if not e.get('stub')]
    exact = [e for e in real if e['matching'] >= 1.0]

    mapped_bytes = 0
    weighted = 0.0
    exact_bytes = 0
    for e in real:
        size = sizes.get(int(e['address'], 16), 0)
        mapped_bytes += size
        weighted += size * e['matching']
        if e['matching'] >= 1.0:
            exact_bytes += size

    def pct(a, b):
        return 100.0 * a / b if b else 0.0

    print('functions compared : %d (%d stubs, %d library)' % (len(funcs), len(stubs), len(library)))
    print('100%% matches       : %d' % len(exact))
    print('mean similarity    : %.1f%% over non-stub functions'
          % pct(sum(e['matching'] for e in real), len(real)))
    print('.text bytes mapped : %#x / %#x (%.1f%%)' % (mapped_bytes, text_size, pct(mapped_bytes, text_size)))
    print('match-weighted     : %.1f%% of .text' % pct(weighted, text_size))
    print('exact bytes        : %.1f%% of .text' % pct(exact_bytes, text_size))
    print('vtables            : %d (%d at 100%%)'
          % (len(vtables), sum(1 for e in vtables if e['matching'] >= 1.0)))


if __name__ == '__main__':
    main()
