set(SQUIRREL223_ROOT "${PROJECT_SOURCE_DIR}/third_party/squirrel223")
set(SQUIRREL223_SOURCES
  ${SQUIRREL223_ROOT}/squirrel/sqapi.cpp
  ${SQUIRREL223_ROOT}/squirrel/sqbaselib.cpp
  ${SQUIRREL223_ROOT}/squirrel/sqclass.cpp
  ${SQUIRREL223_ROOT}/squirrel/sqcompiler.cpp
  ${SQUIRREL223_ROOT}/squirrel/sqdebug.cpp
  ${SQUIRREL223_ROOT}/squirrel/sqfuncstate.cpp
  ${SQUIRREL223_ROOT}/squirrel/sqlexer.cpp
  ${SQUIRREL223_ROOT}/squirrel/sqmem.cpp
  ${SQUIRREL223_ROOT}/squirrel/sqobject.cpp
  ${SQUIRREL223_ROOT}/squirrel/sqstate.cpp
  ${SQUIRREL223_ROOT}/squirrel/sqtable.cpp
  ${SQUIRREL223_ROOT}/squirrel/sqvm.cpp
  ${SQUIRREL223_ROOT}/sqstdlib/sqstdaux.cpp
  ${SQUIRREL223_ROOT}/sqstdlib/sqstdblob.cpp
  ${SQUIRREL223_ROOT}/sqstdlib/sqstdio.cpp
  ${SQUIRREL223_ROOT}/sqstdlib/sqstdrex.cpp
  ${SQUIRREL223_ROOT}/sqstdlib/sqstdstream.cpp
  ${SQUIRREL223_ROOT}/sqstdlib/sqstdstring.cpp)
# sqstdmath.cpp and sqstdsystem.cpp are replaced by the host forks
# src/host/sqstd_math.cpp and src/host/sqstd_system.cpp.
# The game's sqstdio runs on KD files; see src/host/kd_stdio.h.
set_source_files_properties(${SQUIRREL223_ROOT}/sqstdlib/sqstdio.cpp PROPERTIES
  COMPILE_FLAGS "/FIhost/kd_stdio.h")
