"""Serve the web build (build/web/bin) with cross-origin isolation.

The page shares the module's memory with its sound thread
(src/imports/web/audio.cpp), which browsers allow only on pages sent with
COOP/COEP headers; python -m http.server sends none. The headers are the
ones build/web/bin/_headers gives the page on Cloudflare Pages.

    serve-web.py [port]    (default 8000, on 127.0.0.1)
"""
import functools
import http.server
import os
import sys


class Handler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        super().end_headers()


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
    root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'build', 'web', 'bin')
    handler = functools.partial(Handler, directory=root)
    with http.server.ThreadingHTTPServer(('127.0.0.1', port), handler) as server:
        print('Serving %s at http://localhost:%d/' % (os.path.normpath(root), port))
        server.serve_forever()


if __name__ == '__main__':
    main()
