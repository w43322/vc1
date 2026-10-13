# SqPlus, SourceForge sqplus SQUIRREL2_1_1_sqplus_snapshot_20080713, sqplus/
# only (the bundled Squirrel 2.1.1 core is not used; the game links stock
# 2.2.3). SquirrelVM.h/.cpp are not vendored: the engine's rewrite lives in
# src/sqplus and is found through the include path. Engine patches are marked
# "VC1:": SqPlusSetup.h options and hooks, SQPLUS_THROW for every
# `throw SquirrelError(...)` (plus `return SQ_ERROR` in SqPlus.cpp), Reset without the "Cannot release" printf, and the
# engine's DirectCallInstanceMemberFunction::Dispatch.
set(SQPLUS_ROOT "${PROJECT_SOURCE_DIR}/third_party/sqplus")
set(SQPLUS_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src/sqplus" "${SQPLUS_ROOT}")
set(SQPLUS_SOURCES
  ${SQPLUS_ROOT}/SquirrelObject.cpp
  ${SQPLUS_ROOT}/SqPlus.cpp
  ${SQPLUS_ROOT}/SquirrelBindingsUtils.cpp)
