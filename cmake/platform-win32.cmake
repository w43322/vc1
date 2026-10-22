# Matching host backend. Preserve translation-unit ordering for VC9 LTCG.
# A future SDL3 build can supply the same host/platform.h services here.
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
)
