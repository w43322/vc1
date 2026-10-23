cmake_minimum_required(VERSION 3.13)
# Explicit VC9 flags below also work with newer CMake without runtime abstraction.
if(POLICY CMP0091)
  cmake_policy(SET CMP0091 OLD)
endif()
project(VirtualCityDecomp VERSION 0.1.0 LANGUAGES C CXX)

option(VC1_USE_SDL3 "Build the SDL3 backend (Windows x86, modern MSVC)" OFF)
# Addons, not original code. They default to on for the SDL3 build only, so a
# default reference build stays comparable with the original.
option(VC1_MODS "Texture mods from mods/ beside the executable" ${VC1_USE_SDL3})
option(VC1_LOG "Send KdLog text to OutputDebugStringA (SDL3: the console)" ${VC1_USE_SDL3})
if(VC1_USE_SDL3)
  # Normalize redirected cl output so a localized compiler cannot silently lose
  # Ninja header dependencies. This affects the modern target only.
  if(EXISTS "${CMAKE_SOURCE_DIR}/.venv/Scripts/python.exe")
    set(VC1_BUILD_PYTHON "${CMAKE_SOURCE_DIR}/.venv/Scripts/python.exe")
  else()
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(VC1_BUILD_PYTHON "${Python3_EXECUTABLE}")
  endif()
  set(CMAKE_C_COMPILER_LAUNCHER "${VC1_BUILD_PYTHON};${CMAKE_SOURCE_DIR}/tools/msvc-showincludes.py")
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CMAKE_C_COMPILER_LAUNCHER}")
  set(CMAKE_CL_SHOWINCLUDES_PREFIX "Note: including file: ")
  set(CMAKE_C_CL_SHOWINCLUDES_PREFIX "${CMAKE_CL_SHOWINCLUDES_PREFIX}")
  set(CMAKE_CXX_CL_SHOWINCLUDES_PREFIX "${CMAKE_CL_SHOWINCLUDES_PREFIX}")
  if(NOT WIN32 OR NOT MSVC OR MSVC_VERSION LESS 1910)
    message(FATAL_ERROR "The SDL3 build currently requires modern MSVC on Windows x86.")
  endif()
  set(VC1_TARGET vc1_sdl3)
  set(VC1_OUTPUT_NAME "VirtualCity.SDL3")
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
  set(VC1_TARGET vc1_recompiled)
  set(VC1_OUTPUT_NAME "VirtualCity.Unwrapped")
endif()

if(NOT VC1_USE_SDL3 AND (NOT MSVC OR NOT MSVC_VERSION EQUAL 1500))
  message(FATAL_ERROR "Reference builds require VC9 / MSVC 1500. Use tools/build-vc9.cmd.")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR "The reference ABI requires x86 (32-bit pointers).")
endif()
if(NOT CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  message(FATAL_ERROR "Use RelWithDebInfo; this project pins reference optimization/debug flags.")
endif()

# Do not inherit /MD, /Ob1, /RTC or Debug defaults from a CMake release.
# /GS-: the original calls __security_check_cookie (0x0043980a) only from the
# prebuilt CRT; game and library code have no cookies (LoadTgaTexture
# 0x00465770 keeps a 0x200-byte char buffer without one).
set(CMAKE_C_FLAGS "/nologo /W3 /MT /GS-")
set(CMAKE_CXX_FLAGS "/nologo /W3 /MT /GS- /GR-")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "/O2 /Ob2 /Z7 /DNDEBUG /GL")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Ob2 /Z7 /DNDEBUG /GL")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG /INCREMENTAL:NO /OPT:NOREF /OPT:NOICF /SAFESEH:NO /LTCG")
if(VC1_USE_SDL3)
  # Portable branch only: do not change the reference VC9 flags above.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "/O2 /Ob2 /Z7 /DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Ob2 /Z7 /DNDEBUG")
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
include(cmake/squirrel223.cmake)
include(cmake/klibc.cmake)
include(cmake/libtga.cmake)
include(cmake/jpgd.cmake)
include(cmake/libpng.cmake)
include(cmake/lzma.cmake)
include(cmake/pthreads.cmake)
include(cmake/sqplus.cmake)
# The original's DLL imports: the real Windows DLLs, or SDL3 implementations.
if(VC1_USE_SDL3)
  include(cmake/imports-sdl3.cmake)
else()
  include(cmake/imports-win32.cmake)
endif()
include(cmake/kdcore.cmake)

set(VC1_ADDON_SOURCES)
if(VC1_MODS)
  list(APPEND VC1_ADDON_SOURCES src/mod/mods.cpp src/mod/mod_io.cpp src/mod/texture_hook.cpp)
endif()

add_executable(${VC1_TARGET}
  ${VC1_HOST_SOURCES}
  src/app/script.cpp
  src/app/host_services.cpp
  src/app/host_api.cpp
  src/app/run.cpp
  src/kd/iface/iid.cpp
  src/kd/iface/iunknown.cpp
  src/kd/iface/iabstract.cpp
  src/kd/factory.cpp
  src/kd/script_instance.cpp
  src/kd/display/display.cpp
  src/kd/display/graphics.cpp
  src/kd/display/pyro.cpp
  src/kd/display/effect.cpp
  src/pyro/pyro_cfile.cpp
  src/pyro/pyro_emitter.cpp
  src/pyro/pyro_file.cpp
  src/pyro/pyro_image.cpp
  src/pyro/pyro_random.cpp
  src/pyro/pyro_matrix.cpp
  src/pyro/pyro_stream.cpp
  src/pyro/pyro_string.cpp
  src/kd/display/render_target.cpp
  src/kd/display/texture.cpp
  src/kd/system/system.cpp
  src/kd/component_group.cpp
  src/kd/sound/sound_driver.cpp
  src/kd/sound/music.cpp
  src/kd/sound/sound.cpp
  src/kd/scene/compound_object.cpp
  src/kd/console/console.cpp
  src/kd/font/font.cpp
  src/kd/stream/file_reader.cpp
  src/kd/scene/layer.cpp
  src/kd/group/ptr_set.cpp
  src/kd/group/update_group.cpp
  src/kd/group/render_group.cpp
  src/kd/group/input_group.cpp
  src/kd/resource/resource_manager.cpp
  src/kd/file/file.cpp
  src/kd/scene/scene.cpp
  src/kd/blob/blob.cpp
  src/kd/scene/camera.cpp
  src/kd/scene/scripted_object.cpp
  src/kd/world/game_level.cpp
  src/kd/world/game_object.cpp
  src/kd/tile/tile_manager.cpp
  src/kd/tile/tile_animation_data.cpp
  src/kd/tile/tile_layer.cpp
  src/kd/tile/tile_object.cpp
  src/kd/road/pass_map.cpp
  src/kd/road/grid_router.cpp
  src/kd/build/terrain_builder.cpp
  src/kd/road/road_renderer.cpp
  src/kd/tile/tile_animation.cpp
  src/kd/build/building_builder.cpp
  src/kd/route/route.cpp
  src/kd/route/transport_mover.cpp
  src/kd/road/grid_rect_renderer.cpp
  src/kd/render/object_watcher.cpp
  src/kd/render/classificated_renderer.cpp
  src/kd/route/tsp_solver.cpp
  src/kd/render/group_renderer.cpp
  src/kd/road/road_cells_renderer.cpp
  src/bindings/color.cpp
  src/bindings/host_classes.cpp
  src/bindings/vector2.cpp
  src/bindings/vector3.cpp
  src/bindings/vector4.cpp
  src/bindings/matrix3.cpp
  src/bindings/matrix4.cpp
  src/bindings/quaternion.cpp
  src/bindings/rect.cpp
  src/bindings/rectf.cpp
  src/bindings/integral.cpp
  src/bindings/component.cpp
  src/bindings/sqplus.cpp
  src/bindings/squirrel_object.cpp
  src/sqplus/SquirrelVM.cpp
  src/memory/fixed_pool.cpp
  src/memory/global_new.cpp
  ${SQUIRREL223_SOURCES}
  ${KLIBC_SOURCES}
  ${LIBTGA_SOURCES}
  ${JPGD_SOURCES}
  ${ZLIB_SOURCES}
  ${LIBPNG_SOURCES}
  ${LZMA457_SOURCES}
  ${PTW32_SOURCES}
  ${SQPLUS_SOURCES}
  ${VC1_ADDON_SOURCES})
if(VC1_MODS)
  target_compile_definitions(${VC1_TARGET} PRIVATE VC1_MODS=1)
endif()
if(VC1_LOG)
  target_compile_definitions(${VC1_TARGET} PRIVATE VC1_LOG=1)
endif()
if(VC1_USE_SDL3)
  # Ahead of every other directory: the SDK headers resolve to the SDL subset.
  target_include_directories(${VC1_TARGET} BEFORE PRIVATE src/imports/sdl3/include)
endif()
target_include_directories(${VC1_TARGET} PRIVATE src third_party/squirrel223/include
  third_party/squirrel223/squirrel third_party/libtga third_party/jpgd093b third_party/libpng102 third_party/zlib113
  third_party/lzma457/C/Archive/7z third_party/pthreads-w32-2-8-0
  ${SQPLUS_INCLUDE_DIRS})
target_compile_definitions(${VC1_TARGET} PRIVATE _CRT_SECURE_NO_WARNINGS ${PTW32_DEFINITIONS})
set_target_properties(${VC1_TARGET} PROPERTIES OUTPUT_NAME "${VC1_OUTPUT_NAME}")
if(VC1_USE_SDL3)
  # Keep the vendored scanf definitions instead of UCRT's header-inline copies.
  target_compile_definitions(${VC1_TARGET} PRIVATE _NO_CRT_STDIO_INLINE
    sscanf=vc1_klibc_sscanf vsscanf=vc1_klibc_vsscanf)
  get_target_property(VC1_MIXER_TYPE SDL3_mixer::SDL3_mixer TYPE)
  if(VC1_MIXER_TYPE STREQUAL "SHARED_LIBRARY")
    add_custom_command(TARGET ${VC1_TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL3_mixer::SDL3_mixer>
        $<TARGET_FILE_DIR:${VC1_TARGET}>)
  endif()
  add_custom_command(TARGET ${VC1_TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL3::SDL3>
      $<TARGET_FILE_DIR:${VC1_TARGET}>)
endif()

target_link_libraries(${VC1_TARGET} ${VC1_IMPORT_LIBRARIES})
if(VC1_MODS)
  # Texture mods live in the local, git-ignored mods/ folder. Seed the
  # activation list once so rebuilding never re-enables a disabled mod, and
  # refresh every mod folder beside the executable on each build.
  if(EXISTS "${CMAKE_SOURCE_DIR}/mods/enabled.txt" AND
     NOT EXISTS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mods/enabled.txt")
    configure_file(mods/enabled.txt "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mods/enabled.txt" COPYONLY)
  endif()
  file(GLOB VC1_MOD_ENTRIES LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/mods/*")
  set(VC1_MOD_COPY_COMMANDS)
  foreach(VC1_MOD_DIR IN LISTS VC1_MOD_ENTRIES)
    if(IS_DIRECTORY "${VC1_MOD_DIR}")
      get_filename_component(VC1_MOD_NAME "${VC1_MOD_DIR}" NAME)
      list(APPEND VC1_MOD_COPY_COMMANDS COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${VC1_MOD_DIR}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mods/${VC1_MOD_NAME}")
    endif()
  endforeach()
  if(VC1_MOD_COPY_COMMANDS)
    add_custom_target(vc1_mods ALL ${VC1_MOD_COPY_COMMANDS} VERBATIM)
    add_dependencies(${VC1_TARGET} vc1_mods)
  endif()
endif()
if(NOT VC1_USE_SDL3 AND EXISTS "${VC1_GAME_BASS_DLL}")
  add_custom_command(TARGET ${VC1_TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${VC1_GAME_BASS_DLL}"
      "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/BASS.dll")
endif()
# The game loads its cursors (setCursor ids 100..105) and icon from its own
# module, so the original's resources are dumped into the build tree at
# configure time and linked in (local copy only, like BASS.dll).
set(VC1_ORIGINAL_EXE "${CMAKE_SOURCE_DIR}/Virtual-City_Win_EN_Full/VirtualCity.Unwrapped.exe")
set(VC1_PYTHON "${CMAKE_SOURCE_DIR}/.venv/Scripts/python.exe")
if(VC1_USE_SDL3 AND EXISTS "${VC1_ORIGINAL_EXE}" AND EXISTS "${VC1_PYTHON}")
  execute_process(COMMAND "${VC1_PYTHON}" "${CMAKE_SOURCE_DIR}/tools/extract-sdl-resources.py"
    "${VC1_ORIGINAL_EXE}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/platform-resources"
    RESULT_VARIABLE VC1_SDL_RSRC_RESULT)
  if(NOT VC1_SDL_RSRC_RESULT EQUAL 0)
    message(FATAL_ERROR "SDL cursor/icon extraction failed")
  endif()
endif()
if(NOT VC1_USE_SDL3 AND EXISTS "${VC1_ORIGINAL_EXE}" AND EXISTS "${VC1_PYTHON}")
  set(VC1_RSRC_RES "${CMAKE_BINARY_DIR}/original-rsrc/original.res")
  execute_process(COMMAND "${VC1_PYTHON}" "${CMAKE_SOURCE_DIR}/tools/extract-rsrc.py"
    "${VC1_ORIGINAL_EXE}" "${VC1_RSRC_RES}" RESULT_VARIABLE VC1_RSRC_RESULT)
  if(VC1_RSRC_RESULT EQUAL 0)
    target_link_options(${VC1_TARGET} PRIVATE "${VC1_RSRC_RES}")
  endif()
endif()
target_link_options(${VC1_TARGET} PRIVATE
  "/SUBSYSTEM:WINDOWS"
  "/MAP:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${VC1_OUTPUT_NAME}.map")
if(NOT VC1_USE_SDL3)
  configure_file(cmake/reccmp-build.yml.in "${CMAKE_BINARY_DIR}/reccmp-build.yml" @ONLY)
  # Full reccmp pass: decomplint, asmcmp (json/html/svg), round-to-round aggregate,
  # datacmp, vtable, and roadmap. Stack layout is tools/stackcmp-reccmp.ps1 <offset>.
  add_custom_target(compare
    COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/tools/compare-reccmp.ps1"
    DEPENDS ${VC1_TARGET} USES_TERMINAL)
endif()
