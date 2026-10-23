# The recovered OpenKODE core (src/kdcore/), shared by both targets. The order
# is the reference build's translation-unit order; keep it. crt.cpp holds the
# two file driver slots that depend on the target's CRT (imports/<target>/).
set(VC1_HOST_SOURCES
  src/kdcore/win_main.cpp
  src/kdcore/runtime.cpp
  src/kdcore/crash.cpp
  src/kdcore/thread.cpp
  src/kdcore/network.cpp
  src/kdcore/sqstd_math.cpp
  src/kdcore/sqstd_system.cpp
  src/kdcore/startup.cpp
  ${VC1_IMPORTS_DIR}/crt.cpp
  src/kdcore/kd_core.cpp
  src/kdcore/heap.cpp
  src/kdcore/kd_log.cpp
  src/kdcore/window.cpp
  src/kdcore/input.cpp
  ${VC1_IMPORT_SOURCES}
)
