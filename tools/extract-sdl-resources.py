"""Convert original PE cursor/icon DIBs into local SDL ARGB resources.

Only generated build output contains game art. Nothing is embedded in source.
Usage: extract-sdl-resources.py original.exe output-directory
"""
from pathlib import Path
import struct
import sys
import pefile


def extract(exe, destination):
    pe = pefile.PE(str(exe))
    resources = {}
    for kind in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        if kind.id not in (1, 3, 12, 14):
            continue
        for item in kind.directory.entries:
            entry = item.directory.entries[0].data.struct
            resources[kind.id, item.id] = pe.get_data(entry.OffsetToData, entry.Size)
    destination.mkdir(parents=True, exist_ok=True)
    for group_type, item_type, prefix in ((12, 1, "cursor"), (14, 3, "icon")):
        for (kind, ident), group in resources.items():
            if kind != group_type:
                continue
            count = struct.unpack_from("<H", group, 4)[0]
            choices = []
            for index in range(count):
                record = group[6 + index * 14:20 + index * 14]
                resource_id = struct.unpack_from("<H", record, 12)[0]
                data = resources[item_type, resource_id]
                hotspot = struct.unpack_from("<HH", data) if item_type == 1 else (0, 0)
                dib = data[4:] if item_type == 1 else data
                if len(dib) < 40 or struct.unpack_from("<I", dib)[0] != 40:
                    continue
                width, double_height, _, depth = struct.unpack_from("<iiHH", dib, 4)
                if depth != 32:
                    continue
                height = abs(double_height) // 2
                pixels = bytearray(width * height * 4)
                stride = width * 4
                mask_stride = ((width + 31) // 32) * 4
                alpha_present = any(dib[40 + 3:40 + stride * height:4])
                for y in range(height):
                    src_y = height - 1 - y if double_height > 0 else y
                    for x in range(width):
                        start = 40 + src_y * stride + x * 4
                        pixel = bytearray(dib[start:start + 4])
                        if not alpha_present:
                            mask_at = 40 + stride * height + src_y * mask_stride + x // 8
                            masked = mask_at < len(dib) and dib[mask_at] & (0x80 >> (x % 8))
                            pixel[3] = 0 if masked else 255
                        pixels[(y * width + x) * 4:(y * width + x + 1) * 4] = pixel
                choices.append((abs(width - 32), width, height, hotspot, pixels))
            if choices:
                _, width, height, hotspot, pixels = min(choices, key=lambda item: item[0])
                header = struct.pack("<4sIIII", b"VCR1", width, height, *hotspot)
                (destination / f"{prefix}{ident}.bin").write_bytes(header + pixels)


if __name__ == "__main__":
    extract(Path(sys.argv[1]), Path(sys.argv[2]))
