"""Run an exe for N seconds, screenshot all screens, kill it.

usage: run-screenshot.py [out.png] [seconds] [exe]
Defaults: build/shot.png, 8 s, the recompiled exe. Pass
Virtual-City_Win_EN_Full/VirtualCity.Unwrapped.exe to capture the original.
The working directory is always Virtual-City_Win_EN_Full/VirtualCity.
"""
import os
import subprocess
import sys
import time

from PIL import ImageGrab

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(REPO, "build", "shot.png")
wait = float(sys.argv[2]) if len(sys.argv) > 2 else 8.0
exe = sys.argv[3] if len(sys.argv) > 3 else os.path.join(
    REPO, "build", "vc9", "bin", "VirtualCity.Unwrapped.exe")
cwd = os.path.join(REPO, "Virtual-City_Win_EN_Full", "VirtualCity")

p = subprocess.Popen([os.path.abspath(exe)], cwd=cwd)
time.sleep(wait)
img = ImageGrab.grab(all_screens=True)
img.thumbnail((1600, 1600))
img.save(out)
p.kill()
print("saved", out, img.size)
