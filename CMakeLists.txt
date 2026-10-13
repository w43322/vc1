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
set(CMAKE_C_FLAGS "/nologo /W3 /MT /GS")
set(CMAKE_CXX_FLAGS "/nologo /W3 /MT /GS /GR-")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "/O2 /Ob2 /Z7 /DNDEBUG /GL")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Ob2 /Z7 /DNDEBUG /GL")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG /INCREMENTAL:NO /OPT:NOREF /OPT:NOICF /SAFESEH:NO /LTCG")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
include(cmake/squirrel223.cmake)
include(cmake/klibc.cmake)

add_executable(vc1_recompiled
  src/host/win_main.cpp
  src/host/runtime.cpp
  src/host/crash.cpp
  src/host/thread.cpp
  src/host/network.cpp
  src/host/startup.cpp
  src/host/heap.cpp
  src/host/kd_log.cpp
  src/app/script.cpp
  src/app/host_services.cpp
  src/app/run.cpp
  src/kd/iface/iid.cpp
  src/kd/iface/iunknown.cpp
  src/kd/iface/iabstract.cpp
  src/kd/factory.cpp
  src/kd/display/display.cpp
  src/kd/display/graphics.cpp
  src/kd/display/pyro.cpp
  src/kd/display/render_target.cpp
  src/kd/display/texture.cpp
  src/kd/system/system.cpp
  src/kd/component_group.cpp
  src/kd/sound/sound_driver.cpp
  src/kd/scene/compound_object.cpp
  src/kd/console/console.cpp
  src/kd/font/font.cpp
  src/kd/stream/file_reader.cpp
  src/kd/scene/layer.cpp
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
  src/bindings/value_types.cpp
  src/bindings/color.cpp
  src/bindings/vector2.cpp
  src/bindings/vector3.cpp
  src/bindings/vector4.cpp
  src/bindings/matrix3.cpp
  src/bindings/matrix4.cpp
  src/bindings/quaternion.cpp
  src/bindings/rect.cpp
  src/bindings/rectf.cpp
  src/bindings/sqplus.cpp
  src/bindings/squirrel_object.cpp
  src/memory/fixed_pool.cpp
  ${SQUIRREL223_SOURCES}
  ${KLIBC_SOURCES})
target_include_directories(vc1_recompiled PRIVATE src third_party/squirrel223/include
  third_party/squirrel223/squirrel)
target_compile_definitions(vc1_recompiled PRIVATE _CRT_SECURE_NO_WARNINGS)
set_target_properties(vc1_recompiled PROPERTIES OUTPUT_NAME "VirtualCity.Unwrapped")
target_link_libraries(vc1_recompiled user32 ws2_32 shell32)
target_link_options(vc1_recompiled PRIVATE
  "/SUBSYSTEM:WINDOWS"
  "/MAP:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/VirtualCity.Unwrapped.map")
configure_file(cmake/reccmp-build.yml.in "${CMAKE_BINARY_DIR}/reccmp-build.yml" @ONLY)
# Full reccmp pass: decomplint, asmcmp (json/html/svg), round-to-round aggregate,
# datacmp, vtable, and roadmap. Stack layout is tools/stackcmp-reccmp.ps1 <offset>.
add_custom_target(compare
  COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/tools/compare-reccmp.ps1"
  DEPENDS vc1_recompiled USES_TERMINAL)
