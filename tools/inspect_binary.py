"""Read-only PE fingerprint of the original EXE. Writes under build/ by default."""
import argparse
import datetime
import hashlib
import json
import re
from collections import Counter
from pathlib import Path

import pefile

ROOT = Path(__file__).resolve().parents[1]
PRODUCTS = {
    1: 'Import0', 4: 'Linker600', 29: 'Utc13_CPP', 37: 'Linker620',
    123: 'Implib800', 131: 'Utc1500_C', 132: 'Utc1500_CPP',
    138: 'Utc1500_LTCG_CPP', 145: 'Linker900', 148: 'Cvtres900',
    149: 'Masm900', 150: 'AliasObj900',
}

def fingerprint(path):
    data = path.read_bytes()
    return {'path': path.relative_to(ROOT).as_posix(), 'size': len(data),
            'sha256': hashlib.sha256(data).hexdigest()}

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, default=ROOT / 'Virtual-City_Win_EN_Full/VirtualCity.Unwrapped.exe')
    parser.add_argument('--output', type=Path, default=ROOT / 'build/binary-fingerprint.json')
    args = parser.parse_args()
    path = args.exe.resolve()
    data = path.read_bytes()
    pe = pefile.PE(data=data)
    base = pe.OPTIONAL_HEADER.ImageBase
    rich = pe.parse_rich_header()
    vals = rich['values'] if rich else []
    strings = []
    for match in re.finditer(rb'[\x20-\x7e]{6,}', data):
        if re.search(rb'Squirrel|\.pdb$|_charsize_|_intsize_|_floatsize_|collectgarbage|Microsoft Visual C', match[0]):
            strings.append({'offset': hex(match.start()),
                            'va': hex(base + pe.get_rva_from_offset(match.start())),
                            'text': match[0].decode('ascii')})
    script_files = sorted((path.parent / 'VirtualCity/scripts').rglob('*.nut'))
    headers = Counter(p.read_bytes()[:14].hex() for p in script_files)
    out = {
        'input': fingerprint(path), 'machine': hex(pe.FILE_HEADER.Machine),
        'timestamp_utc': datetime.datetime.fromtimestamp(pe.FILE_HEADER.TimeDateStamp, datetime.timezone.utc).isoformat(),
        'image_base': hex(base), 'entry_point': hex(base + pe.OPTIONAL_HEADER.AddressOfEntryPoint),
        'linker_version': [pe.OPTIONAL_HEADER.MajorLinkerVersion, pe.OPTIONAL_HEADER.MinorLinkerVersion],
        'file_characteristics': hex(pe.FILE_HEADER.Characteristics),
        'dll_characteristics': hex(pe.OPTIONAL_HEADER.DllCharacteristics),
        'subsystem': pe.OPTIONAL_HEADER.Subsystem,
        'rich': [{'comp_id': f'{a:08x}', 'product_id': a >> 16, 'product': PRODUCTS.get(a >> 16, 'unknown'),
                  'build': a & 65535, 'count': n} for a, n in zip(vals[::2], vals[1::2])],
        'sections': [{'name': s.Name.rstrip(b'\0').decode(), 'rva': hex(s.VirtualAddress),
                      'raw_offset': hex(s.PointerToRawData), 'raw_size': s.SizeOfRawData,
                      'characteristics': hex(s.Characteristics)} for s in pe.sections],
        'imports': {d.dll.decode(): [(i.name.decode() if i.name else f'ordinal:{i.ordinal}') for i in d.imports]
                    for d in getattr(pe, 'DIRECTORY_ENTRY_IMPORT', [])},
        'delay_imports': [d.dll.decode() for d in getattr(pe, 'DIRECTORY_ENTRY_DELAY_IMPORT', [])],
        'debug': [str(d.entry) for d in getattr(pe, 'DIRECTORY_ENTRY_DEBUG', []) if hasattr(d, 'entry')],
        'strings': strings, 'nut_count': len(script_files), 'nut_header_counts': dict(headers),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(out, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
    print(f'Wrote {args.output}; {len(script_files)} .nut files')

if __name__ == '__main__':
    main()
