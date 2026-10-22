"""Normalize localized MSVC /showIncludes lines for Ninja's dependency parser.

CMake's cached probe can disagree with cl's language/code page when stdout is
redirected. Only the two-label include prefix is normalized; diagnostics keep
their text, and the compiler's exit status is forwarded.
"""
import re
import subprocess
import sys

process = subprocess.run(sys.argv[1:], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
try:
    output = process.stdout.decode("utf-8")
except UnicodeDecodeError:
    output = process.stdout.decode("mbcs", errors="replace")
include = re.compile(r"^[^:\r\n]+:\s*[^:\r\n]+:\s+([A-Za-z]:[\\/].*)$")
for line in output.splitlines():
    match = include.match(line)
    if match:
        line = "Note: including file: " + match.group(1).strip()
    sys.stdout.buffer.write((line + "\n").encode("utf-8"))
sys.exit(process.returncode)
