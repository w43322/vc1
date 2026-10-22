# Matching host backend. Preserve the existing host translation-unit ordering.
# Selected only for the reference target. The SDL3 target supplies its own
# complete host/display sources and does not compile this backend.
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
