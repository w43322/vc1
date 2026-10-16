"""Run an exe for N seconds, screenshot all screens, kill it.

usage: run-screenshot.py [out.png] [seconds] [exe] [--click FX,FY@T ...]
Defaults: build/shot.png, 8 s, the recompiled exe. Pass
Virtual-City_Win_EN_Full/VirtualCity.Unwrapped.exe to capture the original.
The working directory is always Virtual-City_Win_EN_Full/VirtualCity.

--click FX,FY@T left-clicks at T seconds after start, at fractions FX,FY of
the game window's client area (0.84,0.53@12 is PLAY GAME on the main menu).
The window must stay visible (not minimized) for the capture.
"""
import ctypes
import ctypes.wintypes as wt
import os
import subprocess
import sys
import time

from PIL import ImageGrab

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

args = []
clicks = []
argv = sys.argv[1:]
i = 0
while i < len(argv):
    if argv[i] == "--click":
        pos, at = argv[i + 1].split("@")
        fx, fy = pos.split(",")
        clicks.append((float(at), float(fx), float(fy)))
        i += 2
    else:
        args.append(argv[i])
        i += 1
clicks.sort()

out = args[0] if len(args) > 0 else os.path.join(REPO, "build", "shot.png")
wait = float(args[1]) if len(args) > 1 else 8.0
exe = args[2] if len(args) > 2 else os.path.join(
    REPO, "build", "vc9", "bin", "VirtualCity.Unwrapped.exe")
cwd = os.path.join(REPO, "Virtual-City_Win_EN_Full", "VirtualCity")

user32 = ctypes.windll.user32
# Per-monitor v2: ClientToScreen/SetCursorPos then use physical pixels for
# every window, DPI-aware or not.
if not user32.SetProcessDpiAwarenessContext(ctypes.c_void_p(-4)):
    user32.SetProcessDPIAware()


def find_window(pid):
    found = []
    proc = ctypes.WINFUNCTYPE(wt.BOOL, wt.HWND, wt.LPARAM)

    def cb(hwnd, _):
        owner = wt.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(owner))
        if owner.value == pid and user32.IsWindowVisible(hwnd):
            found.append(hwnd)
        return True

    user32.EnumWindows(proc(cb), 0)
    return found[0] if found else None


def click(pid, fx, fy):
    hwnd = find_window(pid)
    if not hwnd:
        print("click: no window")
        return
    rect = wt.RECT()
    user32.GetClientRect(hwnd, ctypes.byref(rect))
    pt = wt.POINT(int(rect.right * fx), int(rect.bottom * fy))
    user32.ClientToScreen(hwnd, ctypes.byref(pt))
    # The game re-reads the real cursor on its timer (KdWindowProc WM_TIMER),
    # so posted messages alone do not move its pointer.
    user32.SetForegroundWindow(hwnd)
    user32.SetCursorPos(pt.x, pt.y)
    time.sleep(0.3)
    user32.mouse_event(0x0002, 0, 0, 0, 0)  # MOUSEEVENTF_LEFTDOWN
    time.sleep(0.1)
    user32.mouse_event(0x0004, 0, 0, 0, 0)  # MOUSEEVENTF_LEFTUP
    print("clicked %.2f,%.2f at screen %d,%d (client %dx%d)" % (
        fx, fy, pt.x, pt.y, rect.right, rect.bottom))


p = subprocess.Popen([os.path.abspath(exe)], cwd=cwd)
start = time.time()
for at, fx, fy in clicks:
    delay = at - (time.time() - start)
    if delay > 0:
        time.sleep(delay)
    click(p.pid, fx, fy)
delay = wait - (time.time() - start)
if delay > 0:
    time.sleep(delay)
img = ImageGrab.grab(all_screens=True)
img.thumbnail((1600, 1600))
img.save(out)
p.kill()
print("saved", out, img.size)
