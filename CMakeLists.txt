cmake_minimum_required(VERSION 3.13)
# Explicit VC9 flags below also work with newer CMake without runtime abstraction.
if(POLICY CMP0091)
  cmake_policy(SET CMP0091 OLD)
endif()
project(VirtualCityDecomp VERSION 0.1.0 LANGUAGES C CXX)

if(NOT MSVC OR NOT MSVC_VERSION EQUAL 1500)
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
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
include(cmake/squirrel223.cmake)
include(cmake/klibc.cmake)
include(cmake/libtga.cmake)
include(cmake/jpgd.cmake)
include(cmake/libpng.cmake)
include(cmake/lzma.cmake)
include(cmake/sqplus.cmake)
include(cmake/bass.cmake)
include(cmake/d3d8.cmake)

add_executable(vc1_recompiled
  src/host/win_main.cpp
  src/host/runtime.cpp
  src/host/crash.cpp
  src/host/thread.cpp
  src/host/network.cpp
  src/host/sqstd_math.cpp
  src/host/sqstd_system.cpp
  src/host/startup.cpp
  src/host/heap.cpp
  src/host/kd_log.cpp
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
  ${SQPLUS_SOURCES})
target_include_directories(vc1_recompiled PRIVATE src third_party/squirrel223/include
  third_party/squirrel223/squirrel third_party/libtga third_party/jpgd093b third_party/libpng102 third_party/zlib113
  ${SQPLUS_INCLUDE_DIRS})
target_compile_definitions(vc1_recompiled PRIVATE _CRT_SECURE_NO_WARNINGS)
set_target_properties(vc1_recompiled PROPERTIES OUTPUT_NAME "VirtualCity.Unwrapped")
target_link_libraries(vc1_recompiled user32 ws2_32 shell32 shlwapi winmm bass_import d3d8_import)
if(EXISTS "${VC1_GAME_BASS_DLL}")
  add_custom_command(TARGET vc1_recompiled POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${VC1_GAME_BASS_DLL}"
      "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/BASS.dll")
endif()
# The game loads its cursors (setCursor ids 100..105) and icon from its own
# module, so the original's resources are dumped into the build tree at
# configure time and linked in (local copy only, like BASS.dll).
set(VC1_ORIGINAL_EXE "${CMAKE_SOURCE_DIR}/Virtual-City_Win_EN_Full/VirtualCity.Unwrapped.exe")
set(VC1_PYTHON "${CMAKE_SOURCE_DIR}/.venv/Scripts/python.exe")
if(EXISTS "${VC1_ORIGINAL_EXE}" AND EXISTS "${VC1_PYTHON}")
  set(VC1_RSRC_RES "${CMAKE_BINARY_DIR}/original-rsrc/original.res")
  execute_process(COMMAND "${VC1_PYTHON}" "${CMAKE_SOURCE_DIR}/tools/extract-rsrc.py"
    "${VC1_ORIGINAL_EXE}" "${VC1_RSRC_RES}" RESULT_VARIABLE VC1_RSRC_RESULT)
  if(VC1_RSRC_RESULT EQUAL 0)
    target_link_options(vc1_recompiled PRIVATE "${VC1_RSRC_RES}")
  endif()
endif()
target_link_options(vc1_recompiled PRIVATE
  "/SUBSYSTEM:WINDOWS"
  "/MAP:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/VirtualCity.Unwrapped.map")
configure_file(cmake/reccmp-build.yml.in "${CMAKE_BINARY_DIR}/reccmp-build.yml" @ONLY)
# Full reccmp pass: decomplint, asmcmp (json/html/svg), round-to-round aggregate,
# datacmp, vtable, and roadmap. Stack layout is tools/stackcmp-reccmp.ps1 <offset>.
add_custom_target(compare
  COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/tools/compare-reccmp.ps1"
  DEPENDS vc1_recompiled USES_TERMINAL)
