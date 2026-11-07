"""Split the web build's game.data into parts a static host accepts.

Cloudflare Pages takes files up to 25 MiB; game.data (VirtualCity.bar and
the cursors, from Emscripten's file_packager) is larger. cmake/web.cmake runs

    split-web-data.py <game.data> <page dir>

which writes <page dir>/game.data.0, game.data.1, ... and game.data.json,
the list of parts that src/imports/web/shell/loader.js fetches and joins.
"""
import json
import os
import sys

PART_SIZE = 20 * 1024 * 1024


def main(source, page_dir):
    names = []
    with open(source, 'rb') as data:
        while True:
            part = data.read(PART_SIZE)
            if not part:
                break
            name = 'game.data.%d' % len(names)
            with open(os.path.join(page_dir, name), 'wb') as out:
                out.write(part)
            names.append(name)
    # Parts left over from a larger package.
    index = len(names)
    while os.path.exists(os.path.join(page_dir, 'game.data.%d' % index)):
        os.remove(os.path.join(page_dir, 'game.data.%d' % index))
        index += 1
    with open(os.path.join(page_dir, 'game.data.json'), 'w') as out:
        json.dump(names, out)


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
