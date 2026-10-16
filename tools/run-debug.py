"""Minimal Win32 debugger for the 32-bit recompiled VirtualCity exe.

usage: run-debug.py [--exe PATH] [--cwd DIR] [--map MAP] [--timeout SEC] [--bp HEX ...] [--keep]
Prints OutputDebugString text, DLL loads, and on a fatal exception the
registers, the faulting code bytes and a symbolized stack scan.
--bp sets int3 breakpoints (recompiled VAs); each hit prints regs + stack and continues.
--keep leaves the process running after the timeout instead of killing it.
"""
import argparse
import bisect
import os
import ctypes
import ctypes.wintypes as wt

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
import os
import re
import sys
import threading
import time

k32 = ctypes.WinDLL("kernel32", use_last_error=True)

DEBUG_ONLY_THIS_PROCESS = 0x2
CREATE_NEW_CONSOLE = 0x10
DBG_CONTINUE = 0x00010002
DBG_EXCEPTION_NOT_HANDLED = 0x80010001
INFINITE = 0xFFFFFFFF

EXCEPTION_DEBUG_EVENT = 1
CREATE_THREAD_DEBUG_EVENT = 2
CREATE_PROCESS_DEBUG_EVENT = 3
EXIT_THREAD_DEBUG_EVENT = 4
EXIT_PROCESS_DEBUG_EVENT = 5
LOAD_DLL_DEBUG_EVENT = 6
UNLOAD_DLL_DEBUG_EVENT = 7
OUTPUT_DEBUG_STRING_EVENT = 8
RIP_EVENT = 9


class EXCEPTION_RECORD(ctypes.Structure):
    pass


EXCEPTION_RECORD._fields_ = [
    ("ExceptionCode", wt.DWORD),
    ("ExceptionFlags", wt.DWORD),
    ("ExceptionRecord", ctypes.c_void_p),
    ("ExceptionAddress", ctypes.c_void_p),
    ("NumberParameters", wt.DWORD),
    ("ExceptionInformation", ctypes.c_size_t * 15),
]


class EXCEPTION_DEBUG_INFO(ctypes.Structure):
    _fields_ = [("ExceptionRecord", EXCEPTION_RECORD), ("dwFirstChance", wt.DWORD)]


class CREATE_PROCESS_DEBUG_INFO(ctypes.Structure):
    _fields_ = [
        ("hFile", wt.HANDLE),
        ("hProcess", wt.HANDLE),
        ("hThread", wt.HANDLE),
        ("lpBaseOfImage", ctypes.c_void_p),
        ("dwDebugInfoFileOffset", wt.DWORD),
        ("nDebugInfoSize", wt.DWORD),
        ("lpThreadLocalBase", ctypes.c_void_p),
        ("lpStartAddress", ctypes.c_void_p),
        ("lpImageName", ctypes.c_void_p),
        ("fUnicode", wt.WORD),
    ]


class CREATE_THREAD_DEBUG_INFO(ctypes.Structure):
    _fields_ = [
        ("hThread", wt.HANDLE),
        ("lpThreadLocalBase", ctypes.c_void_p),
        ("lpStartAddress", ctypes.c_void_p),
    ]


class LOAD_DLL_DEBUG_INFO(ctypes.Structure):
    _fields_ = [
        ("hFile", wt.HANDLE),
        ("lpBaseOfDll", ctypes.c_void_p),
        ("dwDebugInfoFileOffset", wt.DWORD),
        ("nDebugInfoSize", wt.DWORD),
        ("lpImageName", ctypes.c_void_p),
        ("fUnicode", wt.WORD),
    ]


class OUTPUT_DEBUG_STRING_INFO(ctypes.Structure):
    _fields_ = [
        ("lpDebugStringData", ctypes.c_void_p),
        ("fUnicode", wt.WORD),
        ("nDebugStringLength", wt.WORD),
    ]


class EXIT_PROCESS_DEBUG_INFO(ctypes.Structure):
    _fields_ = [("dwExitCode", wt.DWORD)]


class DEBUG_EVENT_UNION(ctypes.Union):
    _fields_ = [
        ("Exception", EXCEPTION_DEBUG_INFO),
        ("CreateThread", CREATE_THREAD_DEBUG_INFO),
        ("CreateProcessInfo", CREATE_PROCESS_DEBUG_INFO),
        ("ExitProcess", EXIT_PROCESS_DEBUG_INFO),
        ("LoadDll", LOAD_DLL_DEBUG_INFO),
        ("DebugString", OUTPUT_DEBUG_STRING_INFO),
        ("pad", ctypes.c_byte * 160),
    ]


class DEBUG_EVENT(ctypes.Structure):
    _fields_ = [
        ("dwDebugEventCode", wt.DWORD),
        ("dwProcessId", wt.DWORD),
        ("dwThreadId", wt.DWORD),
        ("u", DEBUG_EVENT_UNION),
    ]


class WOW64_FLOATING_SAVE_AREA(ctypes.Structure):
    _fields_ = [
        ("ControlWord", wt.DWORD),
        ("StatusWord", wt.DWORD),
        ("TagWord", wt.DWORD),
        ("ErrorOffset", wt.DWORD),
        ("ErrorSelector", wt.DWORD),
        ("DataOffset", wt.DWORD),
        ("DataSelector", wt.DWORD),
        ("RegisterArea", ctypes.c_byte * 80),
        ("Cr0NpxState", wt.DWORD),
    ]


class WOW64_CONTEXT(ctypes.Structure):
    _fields_ = [
        ("ContextFlags", wt.DWORD),
        ("Dr0", wt.DWORD), ("Dr1", wt.DWORD), ("Dr2", wt.DWORD),
        ("Dr3", wt.DWORD), ("Dr6", wt.DWORD), ("Dr7", wt.DWORD),
        ("FloatSave", WOW64_FLOATING_SAVE_AREA),
        ("SegGs", wt.DWORD), ("SegFs", wt.DWORD), ("SegEs", wt.DWORD), ("SegDs", wt.DWORD),
        ("Edi", wt.DWORD), ("Esi", wt.DWORD), ("Ebx", wt.DWORD), ("Edx", wt.DWORD),
        ("Ecx", wt.DWORD), ("Eax", wt.DWORD),
        ("Ebp", wt.DWORD), ("Eip", wt.DWORD), ("SegCs", wt.DWORD), ("EFlags", wt.DWORD),
        ("Esp", wt.DWORD), ("SegSs", wt.DWORD),
        ("ExtendedRegisters", ctypes.c_byte * 512),
    ]


WOW64_CONTEXT_ALL = 0x0001003F


class STARTUPINFOW(ctypes.Structure):
    _fields_ = [
        ("cb", wt.DWORD), ("lpReserved", wt.LPWSTR), ("lpDesktop", wt.LPWSTR),
        ("lpTitle", wt.LPWSTR), ("dwX", wt.DWORD), ("dwY", wt.DWORD),
        ("dwXSize", wt.DWORD), ("dwYSize", wt.DWORD), ("dwXCountChars", wt.DWORD),
        ("dwYCountChars", wt.DWORD), ("dwFillAttribute", wt.DWORD), ("dwFlags", wt.DWORD),
        ("wShowWindow", wt.WORD), ("cbReserved2", wt.WORD), ("lpReserved2", ctypes.c_void_p),
        ("hStdInput", wt.HANDLE), ("hStdOutput", wt.HANDLE), ("hStdError", wt.HANDLE),
    ]


class PROCESS_INFORMATION(ctypes.Structure):
    _fields_ = [
        ("hProcess", wt.HANDLE), ("hThread", wt.HANDLE),
        ("dwProcessId", wt.DWORD), ("dwThreadId", wt.DWORD),
    ]


k32.WaitForDebugEvent.argtypes = [ctypes.POINTER(DEBUG_EVENT), wt.DWORD]
k32.ContinueDebugEvent.argtypes = [wt.DWORD, wt.DWORD, wt.DWORD]
k32.ReadProcessMemory.argtypes = [wt.HANDLE, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t,
                                  ctypes.POINTER(ctypes.c_size_t)]
k32.WriteProcessMemory.argtypes = [wt.HANDLE, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t,
                                   ctypes.POINTER(ctypes.c_size_t)]
k32.FlushInstructionCache.argtypes = [wt.HANDLE, ctypes.c_void_p, ctypes.c_size_t]
k32.Wow64GetThreadContext.argtypes = [wt.HANDLE, ctypes.POINTER(WOW64_CONTEXT)]
k32.Wow64SetThreadContext.argtypes = [wt.HANDLE, ctypes.POINTER(WOW64_CONTEXT)]
k32.GetFinalPathNameByHandleW.argtypes = [wt.HANDLE, wt.LPWSTR, wt.DWORD, wt.DWORD]
k32.TerminateProcess.argtypes = [wt.HANDLE, wt.UINT]
k32.OpenThread.restype = wt.HANDLE
k32.CreateProcessW.argtypes = [wt.LPCWSTR, wt.LPWSTR, ctypes.c_void_p, ctypes.c_void_p, wt.BOOL,
                               wt.DWORD, ctypes.c_void_p, wt.LPCWSTR,
                               ctypes.POINTER(STARTUPINFOW), ctypes.POINTER(PROCESS_INFORMATION)]


class Symbols:
    def __init__(self):
        self.addrs = []
        self.names = []
        self.mods = []  # (base, end, name)

    def load_map(self, path, base_delta=0):
        pat = re.compile(r"^\s*[0-9a-fA-F]{4}:[0-9a-fA-F]{8}\s+(\S+)\s+([0-9a-fA-F]{8})\s")
        pairs = []
        if not os.path.exists(path):
            print("== no map file %s; addresses stay unsymbolized" % path)
            return
        with open(path, "r", errors="replace") as f:
            for line in f:
                m = pat.match(line)
                if m:
                    pairs.append((int(m.group(2), 16) + base_delta, m.group(1)))
        pairs.sort()
        self.addrs = [a for a, _ in pairs]
        self.names = [n for _, n in pairs]

    def lookup(self, addr):
        if addr < 0x10000:
            return None
        for base, end, name in self.mods:
            if base <= addr < end and not name.lower().endswith(".exe"):
                return "%s+0x%x" % (name, addr - base)
        i = bisect.bisect_right(self.addrs, addr) - 1
        if i >= 0 and addr - self.addrs[i] < 0x4000:
            return "%s+0x%x" % (self.names[i], addr - self.addrs[i])
        return None


def read(hproc, addr, size):
    buf = ctypes.create_string_buffer(size)
    n = ctypes.c_size_t(0)
    if not k32.ReadProcessMemory(hproc, ctypes.c_void_p(addr), buf, size, ctypes.byref(n)):
        return None
    return buf.raw[: n.value]


def write(hproc, addr, data):
    n = ctypes.c_size_t(0)
    ok = k32.WriteProcessMemory(hproc, ctypes.c_void_p(addr), data, len(data), ctypes.byref(n))
    k32.FlushInstructionCache(hproc, ctypes.c_void_p(addr), len(data))
    return ok


def read_cstr(hproc, addr, limit=1024):
    out = b""
    while len(out) < limit:
        chunk = read(hproc, addr + len(out), 64)
        if not chunk:
            break
        i = chunk.find(b"\0")
        if i >= 0:
            return (out + chunk[:i]).decode("latin-1")
        out += chunk
    return out.decode("latin-1")


FMT = re.compile(r"%([-+ #0]*)(\d+|\*)?(?:\.(\d+|\*))?(hh|h|ll|l|I64|L)?([diouxXcsSpfgeEG%])")


def format_remote(hproc, fmt, argp):
    """printf emulation over a 32-bit cdecl va_list at argp."""
    def take():
        nonlocal argp
        v = read(hproc, argp, 4) or b"\0\0\0\0"
        argp += 4
        return int.from_bytes(v, "little")

    def repl(m):
        flags, width, prec, length, conv = m.groups()
        if conv == "%":
            return "%"
        if width == "*":
            width = str(take())
        if prec == "*":
            prec = str(take())
        spec = "%" + (flags or "") + (width or "") + ("." + prec if prec else "")
        if conv in "fgeEG":
            import struct
            lo, hi = take(), take()
            return (spec + conv) % struct.unpack("<d", struct.pack("<II", lo, hi))[0]
        if length in ("ll", "I64"):
            v = take() | (take() << 32)
        else:
            v = take()
        if conv in "s":
            return (spec + "s") % (read_cstr(hproc, v) if v else "(null)")
        if conv == "c":
            return chr(v & 0xff)
        if conv == "p":
            return "%08X" % v
        if conv in "di" and v & 0x80000000 and length not in ("ll", "I64"):
            v -= 1 << 32
        return (spec + conv.replace("u", "d")) % v

    try:
        return FMT.sub(repl, fmt)
    except Exception as e:  # noqa
        return fmt + "  <format error %s>" % e


def handle_path(h):
    buf = ctypes.create_unicode_buffer(1024)
    if h and k32.GetFinalPathNameByHandleW(h, buf, 1024, 0):
        return buf.value.replace("\\\\?\\", "")
    return "?"


def dump(hproc, hthread, syms, title):
    ctx = WOW64_CONTEXT()
    ctx.ContextFlags = WOW64_CONTEXT_ALL
    if not k32.Wow64GetThreadContext(hthread, ctypes.byref(ctx)):
        print("  (no context, err %d)" % ctypes.get_last_error())
        return None
    print("== %s" % title)
    print("  eip=%08x %s" % (ctx.Eip, syms.lookup(ctx.Eip) or ""))
    print("  eax=%08x ebx=%08x ecx=%08x edx=%08x esi=%08x edi=%08x" %
          (ctx.Eax, ctx.Ebx, ctx.Ecx, ctx.Edx, ctx.Esi, ctx.Edi))
    print("  esp=%08x ebp=%08x efl=%08x" % (ctx.Esp, ctx.Ebp, ctx.EFlags))
    code = read(hproc, ctx.Eip, 16)
    if code:
        print("  code: " + code.hex(" "))
    stack = read(hproc, ctx.Esp, 4 * 256) or b""
    print("  stack scan (return-address candidates):")
    shown = 0
    for i in range(0, len(stack) - 3, 4):
        v = int.from_bytes(stack[i:i + 4], "little")
        s = syms.lookup(v)
        if s:
            print("    [esp+%03x] %08x %s" % (i, v, s))
            shown += 1
            if shown >= 40:
                break
    return ctx


def arm_watch(hthread, addr):
    """4-byte hardware write watchpoint in DR0."""
    ctx = WOW64_CONTEXT()
    ctx.ContextFlags = WOW64_CONTEXT_ALL
    if not k32.Wow64GetThreadContext(hthread, ctypes.byref(ctx)):
        return
    ctx.Dr0 = addr
    ctx.Dr7 = (ctx.Dr7 & ~0x000F0003) | 0x000D0001
    k32.Wow64SetThreadContext(hthread, ctypes.byref(ctx))


def start_clicks(pid, specs):
    """Real cursor clicks from a side thread (the game re-reads GetCursorPos on
    its WM_TIMER, so posted mouse messages do not move its pointer)."""
    user32 = ctypes.windll.user32
    if not user32.SetProcessDpiAwarenessContext(ctypes.c_void_p(-4)):
        user32.SetProcessDPIAware()
    clicks = []
    for s in specs:
        pos, at = s.split("@")
        fx, fy = pos.split(",")
        clicks.append((float(at), float(fx), float(fy)))
    clicks.sort()
    t0 = time.time()

    def find_window():
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

    def run():
        for at, fx, fy in clicks:
            delay = at - (time.time() - t0)
            if delay > 0:
                time.sleep(delay)
            hwnd = find_window()
            if not hwnd:
                print("== click: no window")
                continue
            rect = wt.RECT()
            user32.GetClientRect(hwnd, ctypes.byref(rect))
            pt = wt.POINT(int(rect.right * fx), int(rect.bottom * fy))
            user32.ClientToScreen(hwnd, ctypes.byref(pt))
            user32.SetForegroundWindow(hwnd)
            user32.SetCursorPos(pt.x, pt.y)
            time.sleep(0.3)
            user32.mouse_event(0x0002, 0, 0, 0, 0)
            time.sleep(0.1)
            user32.mouse_event(0x0004, 0, 0, 0, 0)
            print("== click %.2f,%.2f at %.1fs (screen %d,%d)" % (fx, fy, time.time() - t0, pt.x, pt.y))

    threading.Thread(target=run, daemon=True).start()


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", default=os.path.join(REPO, "build", "vc9", "bin", "VirtualCity.Unwrapped.exe"))
    ap.add_argument("--cwd", default=os.path.join(REPO, "Virtual-City_Win_EN_Full", "VirtualCity"))
    ap.add_argument("--map", default=None)
    ap.add_argument("--timeout", type=float, default=20.0)
    ap.add_argument("--bp", nargs="*", default=[])
    ap.add_argument("--bp-max", type=int, default=5, help="hits printed per breakpoint")
    ap.add_argument("--keep", action="store_true")
    ap.add_argument("--nolog", action="store_true", help="do not trace vc1::KdLog")
    ap.add_argument("--watch", default=None, help="recompiled VA: hardware write watchpoint (4 bytes)")
    ap.add_argument("--watch-ptr", default=None,
                    help="recompiled VA of a pointer; each --bp hit re-arms --watch on its target")
    ap.add_argument("--watch-ofs", type=lambda s: int(s, 0), default=0,
                    help="signed offset added to the --watch-ptr target")
    ap.add_argument("--watch-max", type=int, default=20, help="hits printed for --watch")
    ap.add_argument("--bp-args", type=int, default=0,
                    help="instead of a full dump, print N dwords at the first stack argument on every hit")
    ap.add_argument("--click", action="append", default=[],
                    help="FX,FY@T: left-click at T s, fractions of the game client area")
    ap.add_argument("args", nargs="*")
    a = ap.parse_args()
    sys.stdout.reconfigure(errors="backslashreplace")
    mapfile = a.map or os.path.splitext(a.exe)[0] + ".map"

    si = STARTUPINFOW()
    si.cb = ctypes.sizeof(si)
    pi = PROCESS_INFORMATION()
    cmd = ctypes.create_unicode_buffer('"%s" %s' % (a.exe, " ".join(a.args)))
    if not k32.CreateProcessW(a.exe, cmd, None, None, False, DEBUG_ONLY_THIS_PROCESS, None, a.cwd,
                              ctypes.byref(si), ctypes.byref(pi)):
        print("CreateProcess failed", ctypes.get_last_error())
        return 1

    if a.click:
        start_clicks(pi.dwProcessId, a.click)
    syms = Symbols()
    threads = {pi.dwThreadId: pi.hThread}
    hproc = pi.hProcess
    bps = {}  # addr -> original byte
    bp_hits = {}
    pending_rearm = {}  # tid -> addr
    start = time.time()
    ev = DEBUG_EVENT()
    seen_loader_bp = False
    exit_code = None
    log_addr = None
    watch_addr = None
    watch_hits = 0
    while True:
        if time.time() - start > a.timeout:
            print("== timeout after %.0fs; process still running" % a.timeout)
            for tid, h in threads.items():
                dump(hproc, h, syms, "thread %d at timeout" % tid)
            if not a.keep:
                k32.TerminateProcess(hproc, 0xdead)
            break
        if not k32.WaitForDebugEvent(ctypes.byref(ev), 500):
            continue
        code = ev.dwDebugEventCode
        status = DBG_CONTINUE
        if code == CREATE_PROCESS_DEBUG_EVENT:
            info = ev.u.CreateProcessInfo
            base = info.lpBaseOfImage or 0
            delta = base - 0x400000
            syms.load_map(mapfile, delta)
            syms.mods.append((base, base + 0x1000000, os.path.basename(a.exe)))
            print("== process base %08x (map delta %+x)" % (base, delta))
            log_addr = None
            if not a.nolog and "?KdLog@vc1@@YAXPBDZZ" in syms.names:
                log_addr = syms.addrs[syms.names.index("?KdLog@vc1@@YAXPBDZZ")]
                orig = read(hproc, log_addr, 1)
                if orig is not None and write(hproc, log_addr, b"\xcc"):
                    bps[log_addr] = orig
                    bp_hits[log_addr] = 0
            for s in a.bp:
                addr = int(s, 16) + delta
                orig = read(hproc, addr, 1)
                if orig is not None and write(hproc, addr, b"\xcc"):
                    bps[addr] = orig
                    bp_hits[addr] = 0
                    print("  bp %08x %s" % (addr, syms.lookup(addr) or ""))
            if a.watch:
                watch_addr = int(a.watch, 16) + delta
                arm_watch(pi.hThread, watch_addr)
                print("  watch %08x %s" % (watch_addr, syms.lookup(watch_addr) or ""))
        elif code == CREATE_THREAD_DEBUG_EVENT:
            threads[ev.dwThreadId] = ev.u.CreateThread.hThread
            if watch_addr is not None:
                arm_watch(ev.u.CreateThread.hThread, watch_addr)
        elif code == EXIT_THREAD_DEBUG_EVENT:
            threads.pop(ev.dwThreadId, None)
        elif code == LOAD_DLL_DEBUG_EVENT:
            info = ev.u.LoadDll
            path = handle_path(info.hFile)
            name = os.path.basename(path)
            base = info.lpBaseOfDll or 0
            syms.mods.append((base, base + 0x200000, name))
            if "windows" not in path.lower():
                print("  dll %08x %s" % (base, path))
        elif code == OUTPUT_DEBUG_STRING_EVENT:
            info = ev.u.DebugString
            raw = read(hproc, info.lpDebugStringData, info.nDebugStringLength) or b""
            if info.fUnicode:
                text = raw.decode("utf-16-le", "replace")
            else:
                text = raw.decode("latin-1")
            sys.stdout.write("  dbg: " + text.rstrip("\x00").rstrip() + "\n")
        elif code == EXCEPTION_DEBUG_EVENT:
            rec = ev.u.Exception.ExceptionRecord
            exc = rec.ExceptionCode
            addr = rec.ExceptionAddress or 0
            first = ev.u.Exception.dwFirstChance
            hthread = threads.get(ev.dwThreadId) or k32.OpenThread(0x1FFFFF, False, ev.dwThreadId)
            if exc in (0x80000003, 0x4000001F):
                if addr in bps:
                    bp_hits[addr] += 1
                    ctx = WOW64_CONTEXT()
                    ctx.ContextFlags = WOW64_CONTEXT_ALL
                    k32.Wow64GetThreadContext(hthread, ctypes.byref(ctx))
                    if addr == log_addr:
                        fmtp = int.from_bytes(read(hproc, ctx.Esp + 4, 4), "little")
                        msg = format_remote(hproc, read_cstr(hproc, fmtp), ctx.Esp + 8)
                        if not msg.strip():
                            ret = int.from_bytes(read(hproc, ctx.Esp, 4), "little")
                            msg = "<empty fmt %08x %r from %s>" % (fmtp, read(hproc, fmtp, 16), syms.lookup(ret))
                        sys.stdout.write("  log: " + msg.rstrip("\n") + "\n")
                    elif a.bp_args:
                        # compact: first stack arg as a pointer, dump N dwords there
                        argp = int.from_bytes(read(hproc, ctx.Esp + 4, 4) or b"\0\0\0\0", "little")
                        raw = read(hproc, argp, 4 * a.bp_args) or b""
                        words = [int.from_bytes(raw[i:i + 4], "little") for i in range(0, len(raw), 4)]
                        print("== bp %08x hit %d ecx=%08x [arg0=%08x] %s" % (
                            addr, bp_hits[addr], ctx.Ecx, argp, " ".join("%08x" % w for w in words)))
                    elif bp_hits[addr] <= a.bp_max:
                        dump(hproc, hthread, syms, "breakpoint %08x hit %d" % (addr, bp_hits[addr]))
                    if a.watch_ptr and addr != log_addr:
                        target = int.from_bytes(read(hproc, int(a.watch_ptr, 16) + delta, 4) or b"\0\0\0\0",
                                                "little")
                        if target:
                            watch_addr = (target + a.watch_ofs) & 0xffffffff
                            for h in threads.values():
                                arm_watch(h, watch_addr)
                            k32.Wow64GetThreadContext(hthread, ctypes.byref(ctx))
                            print("  watch re-armed on %08x (ptr %08x, bytes %s)" % (
                                watch_addr, target, (read(hproc, watch_addr, 16) or b"").hex(" ", 4)))
                    ctx.Eip = addr
                    ctx.EFlags |= 0x100  # single step to re-arm
                    k32.Wow64SetThreadContext(hthread, ctypes.byref(ctx))
                    write(hproc, addr, bps[addr])
                    pending_rearm[ev.dwThreadId] = addr
                elif not seen_loader_bp or addr >= 0x70000000:
                    seen_loader_bp = True
                else:
                    dump(hproc, hthread, syms, "int3 at %08x" % addr)
            elif exc in (0x80000004, 0x4000001E):
                re_addr = pending_rearm.pop(ev.dwThreadId, None)
                ctx = WOW64_CONTEXT()
                ctx.ContextFlags = WOW64_CONTEXT_ALL
                k32.Wow64GetThreadContext(hthread, ctypes.byref(ctx))
                watched = watch_addr is not None and bool(ctx.Dr6 & 1)
                if watched:
                    watch_hits += 1
                    if watch_hits <= a.watch_max:
                        value = int.from_bytes(read(hproc, watch_addr, 4) or b"\0\0\0\0", "little")
                        dump(hproc, hthread, syms, "watch %08x write %d, now %08x" % (
                            watch_addr, watch_hits, value))
                    ctx.Dr6 = 0
                    k32.Wow64SetThreadContext(hthread, ctypes.byref(ctx))
                if re_addr is not None:
                    write(hproc, re_addr, b"\xcc")
                elif not watched:
                    status = DBG_EXCEPTION_NOT_HANDLED
            elif exc == 0x406D1388 or exc == 0xE06D7363:
                status = DBG_EXCEPTION_NOT_HANDLED
            else:
                info = ""
                if exc == 0xC0000005 and rec.NumberParameters >= 2:
                    kind = {0: "read", 1: "write", 8: "execute"}.get(rec.ExceptionInformation[0], "?")
                    info = " (%s of %08x)" % (kind, rec.ExceptionInformation[1])
                title = "%s exception %08x at %08x%s" % (
                    "first-chance" if first else "SECOND-CHANCE", exc, addr, info)
                dump(hproc, hthread, syms, title)
                status = DBG_EXCEPTION_NOT_HANDLED
                if not first:
                    k32.TerminateProcess(hproc, exc)
        elif code == EXIT_PROCESS_DEBUG_EVENT:
            exit_code = ev.u.ExitProcess.dwExitCode
            print("== process exited with %08x" % exit_code)
            k32.ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, DBG_CONTINUE)
            break
        k32.ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, status)
    return 0


if __name__ == "__main__":
    sys.exit(main())
