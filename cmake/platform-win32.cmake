# Matching host backend. Preserve the existing host translation-unit ordering.
# Selected only for the reference target; the src/host/ entries are the
# recovered host sources the SDL3 target shares (it lists them itself).
set(VC1_HOST_SOURCES
  src/platform/win32/win_main.cpp
  src/platform/win32/runtime.cpp
  src/platform/win32/crash.cpp
  src/platform/win32/thread.cpp
  src/platform/win32/network.cpp
  src/host/sqstd_math.cpp
  src/platform/win32/sqstd_system.cpp
  src/platform/win32/startup.cpp
  src/host/kd_core.cpp
  src/host/heap.cpp
  src/host/kd_log.cpp
  src/platform/win32/window.cpp
  src/platform/win32/input.cpp
)
