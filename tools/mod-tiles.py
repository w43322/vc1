"""Inspect/export a static tile locally; never ship the extracted game artwork.

Reads the existing decompiled Tiles.nut for names and the game's Tiles.bin/PVR
for geometry and pixels. Rendering/runtime mods do not need this Python tool.
"""
import argparse
import json
import re
import struct
from pathlib import Path


class Tiles:
    def __init__(self, game, symbols):
        self.game = game
        text = symbols.read_text(encoding="utf-8-sig")
        self.names = dict((name, int(value)) for name, value in re.findall(
            r"^\s*([A-Z][A-Z0-9_]*)\s*=\s*(-?\d+)", text, re.M))
        images = re.search(r"this\.TileImages\s*<-\s*\[(.*?)\]", text, re.S)
        if not images:
            raise ValueError("Missing TileImages in decompiled Tiles.nut")
        self.images = re.findall(r'"([^"\r\n]+)"', images[1])
        self.data = (game / "res/Tiles.bin").read_bytes()
        self.count, = struct.unpack_from("<H", self.data)
        self.base = 2 + self.count * 12

    def record(self, tile):
        if not 0 <= tile < self.count:
            raise ValueError(f"Invalid tile {tile}")
        x, y, w, h, end = struct.unpack_from("<hhhhI", self.data, 2 + tile * 12)
        start = struct.unpack_from("<I", self.data, 2 + tile * 12 - 4)[0] if tile else 0
        if w <= 0 or h <= 0 or start > end or self.base + end * 2 > len(self.data):
            raise ValueError(f"Invalid tile record {tile}")
        return x, y, w, h, start, end

    def fragments(self, tile, ox, oy, depth=0):
        if depth > 16:
            raise ValueError("Tile recursion limit exceeded")
        x, y, w, h, start, end = self.record(tile)
        code = struct.unpack_from("<" + "H" * (end - start), self.data, self.base + start * 2)
        i = 0
        pen_x = pen_y = 0
        signed = lambda v: v - 65536 if v >= 32768 else v
        while i < len(code):
            command = code[i]
            i += 1
            op = command & 63
            if op == 0:
                atlas = ((command & 192) << 2) | (command >> 8)
                u, v = code[i:i + 2]
                i += 2
                yield dict(atlas="res/" + self.images[atlas], u=signed(u), v=signed(v),
                           x=ox + x, y=oy + y, width=w, height=h)
            elif op == 3:
                pen_x, pen_y = map(signed, code[i:i + 2])
                i += 2
            elif op == 4:
                yield from self.fragments(code[i], ox + pen_x, oy + pen_y, depth + 1)
                i += 1
            else:
                raise ValueError(f"Unsupported opcode {op}: only static unflipped tiles are supported")

    def describe(self, name):
        tile = self.names[name] if name in self.names else int(name)
        x, y, w, h, _, _ = self.record(tile)
        fragments = list(self.fragments(tile, -x, -y))
        for f in fragments:
            if min(f["x"], f["y"], f["u"], f["v"]) < 0 or f["x"] + f["width"] > w or f["y"] + f["height"] > h:
                raise ValueError("Fragment outside logical canvas")
        return dict(name=name, id=tile, width=w, height=h, fragments=fragments)

    def export(self, info, destination):
        from PIL import Image
        output = Image.new("RGBA", (info["width"], info["height"]))
        atlases = {}
        for f in info["fragments"]:
            path = f["atlas"]
            if path not in atlases:
                data = (self.game / path).read_bytes()
                header = struct.unpack_from("<13I", data)
                size, h, w = header[:3]
                if header[6] != 32 or size < 52 or len(data) != size + w * h * 4:
                    raise ValueError(f"Unsupported PVR layout: {path}")
                atlases[path] = Image.frombytes("RGBA", (w, h), data[size:], "raw", "BGRA")
            atlas = atlases[path]
            u, v, w, h = (f[k] for k in ("u", "v", "width", "height"))
            if u + w > atlas.width or v + h > atlas.height:
                raise ValueError(f"Fragment outside atlas: {path}")
            fragment = atlas.crop((u, v, u + w, v + h))
            output.alpha_composite(fragment, (f["x"], f["y"]))
        if destination.exists():
            raise ValueError(f"Refusing to overwrite {destination}")
        destination.parent.mkdir(parents=True, exist_ok=True)
        output.save(destination)


def main():
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("tile", help="Tile constant name or numeric ID")
    parser.add_argument("--export", type=Path, help="Export assembled original tile as PNG (requires Pillow)")
    parser.add_argument("--game", type=Path, default=root / "Virtual-City_Win_EN_Full/VirtualCity")
    parser.add_argument("--symbols", type=Path, default=root / "Virtual-City_Win_EN_Full/nut-src/res/Tiles.nut")
    args = parser.parse_args()
    try:
        tiles = Tiles(args.game, args.symbols)
        info = tiles.describe(args.tile)
        print(json.dumps(info, indent=2))
        if args.export:
            tiles.export(info, args.export)
            print(f"Exported locally: {args.export}")
    except (OSError, ValueError, IndexError, struct.error) as error:
        parser.exit(1, f"mod-tiles: {error}\n")


if __name__ == "__main__":
    main()
