# Matching host backend. Preserve the existing host translation-unit ordering.
# Both targets currently use this window/input backend; SDL3 replaces selected
# services (entry, clock and gamepad), not the native window or D3D8 renderer.
set(VC1_HOST_SOURCES
  src/platform/win32/win_main.cpp
  src/platform/win32/runtime.cpp
  src/platform/win32/crash.cpp
  src/platform/win32/thread.cpp
  src/platform/win32/network.cpp
  src/host/sqstd_math.cpp
  src/platform/win32/sqstd_system.cpp
  src/platform/win32/startup.cpp
  src/platform/win32/heap.cpp
  src/platform/win32/kd_log.cpp
  src/platform/win32/window.cpp
  src/platform/win32/input.cpp
)
