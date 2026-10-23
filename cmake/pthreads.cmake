# pthreads-win32 2.8.0 (https://sourceware.org/pub/pthreads-win32/
# pthreads-w32-2-8-0-release.tar.gz, SHA-256 ff7a2874b04581ed344f7d3bb0548d77
# 98891216eebcee830887b37c85ab48fd), upstream files unchanged. The original
# links it like the upstream "nmake VC-static" target: the inlined pthread.c
# with C cleanup (ptw32_threadStart 0x0047b600 calls setjmp around the
# routine) into a static library, compiled with the game under /GL.
set(PTW32_ROOT "${PROJECT_SOURCE_DIR}/third_party/pthreads-w32-2-8-0")
set(PTW32_SOURCES ${PTW32_ROOT}/pthread.c)
set(PTW32_DEFINITIONS PTW32_STATIC_LIB __CLEANUP_C)
set_source_files_properties(${PTW32_SOURCES} PROPERTIES
  COMPILE_DEFINITIONS "PTW32_BUILD_INLINED;HAVE_CONFIG_H;_WIN32_WINNT=0x400")
if(VC1_USE_SDL3)
  # UCRT's <time.h> already defines struct timespec, and the SDL3 SDK subset
  # (imports/sdl3/include) is C++, so the library builds as C++ there.
  list(APPEND PTW32_DEFINITIONS HAVE_STRUCT_TIMESPEC)
  # config.h undefines HAVE_STRUCT_TIMESPEC for pthread.c, so UCRT's copy is
  # kept out of that file (it uses no time_t); C++ needs <eh.h> for the
  # destructor guard in ptw32_callUserDestroyRoutines.
  set_property(SOURCE ${PTW32_SOURCES} APPEND PROPERTY COMPILE_OPTIONS /TP /FIeh.h)
  set_property(SOURCE ${PTW32_SOURCES} APPEND PROPERTY COMPILE_DEFINITIONS _CRT_NO_TIME_T)
endif()
