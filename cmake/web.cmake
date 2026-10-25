# Web target: the SDL3 build compiled to WebAssembly by Emscripten
# (tools/build-web.cmd). The recovered sources are unchanged; what the browser
# needs beyond SDL3 lives in src/imports/web/: MSVC CRT names (force-included
# vc1_web_crt.h and the include/ stand-ins), the entry point that calls
# WinMain, and the page (shell/) that plays the packaged VirtualCity.bar.
#
# The game's main loop never returns, so the module is built with ASYNCIFY:
# Present and Sleep yield to the browser (src/imports/web/web.cpp).
set(VC1_WEB_DIR "${CMAKE_SOURCE_DIR}/src/imports/web")

# clang in MSVC-extension mode reads the VC9-style sources (__declspec,
# __forceinline, __int64, #pragma function); __stdcall has no meaning on wasm.
set(VC1_WEB_COMMON_FLAGS "-fms-extensions -fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks -Wno-ignored-attributes -Wno-microsoft -Wno-unused-value -Wno-deprecated-declarations -Wno-pragma-pack -Wno-register")
set(CMAKE_C_FLAGS "${VC1_WEB_COMMON_FLAGS}")
# VC9 parses templates only when they are instantiated (SqPlus, Squirrel).
set(CMAKE_CXX_FLAGS "${VC1_WEB_COMMON_FLAGS} -fno-rtti -fdelayed-template-parsing")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -gline-tables-only -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -gline-tables-only -DNDEBUG")
# The link runs binaryen at -O0 and adds -O2 after Asyncify instead: an
# optimizing link runs --post-emscripten after --fpcast-emu, and that pass
# turns the invoke_* calls of setjmp functions (libpng, Squirrel) into direct
# calls of the cast-emulation thunks, which fails validation.
# wasm-ld keeps -O2: it merges identical string literals across objects as
# MSVC's string pooling does, and the KD factory compares interned name
# pointers.
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-O0 -sASSERTIONS=0 -sBINARYEN_EXTRA_PASSES=-O2 -Wl,-O2 -gsource-map")

# The library cmake files give MSVC per-source options (/FI, /TP, /wd...,
# /Zc:forScope-). Translate them for clang once every source is known.
function(vc1_web_translate_source source)
  foreach(property COMPILE_FLAGS COMPILE_OPTIONS)
    get_source_file_property(value "${source}" ${property})
    if(value)
      string(REGEX REPLACE "/FIeh\\.h" "" value "${value}")
      string(REGEX REPLACE "/FI" "-include" value "${value}")
      if(value MATCHES "/TP")
        string(REPLACE "/TP" "" value "${value}")
        set_source_files_properties("${source}" PROPERTIES LANGUAGE CXX)
      endif()
      string(REGEX REPLACE "/wd[0-9]+" "" value "${value}")
      string(REPLACE "/Zc:forScope-" "" value "${value}")
      set_source_files_properties("${source}" PROPERTIES ${property} "${value}")
    endif()
  endforeach()
  # klibc's vc9/ stand-ins for <stdint.h> / <inttypes.h> are VC9's; musl has both.
  get_source_file_property(dirs "${source}" INCLUDE_DIRECTORIES)
  if(dirs)
    list(FILTER dirs EXCLUDE REGEX "/vc9$")
    set_source_files_properties("${source}" PROPERTIES INCLUDE_DIRECTORIES "${dirs}")
  endif()
endfunction()

function(vc1_web_target target)
  get_target_property(sources ${target} SOURCES)
  foreach(source IN LISTS sources)
    vc1_web_translate_source("${source}")
  endforeach()
  # pthreads-win32's <pthread.h> would hide musl's from libc++ as well.
  get_target_property(dirs ${target} INCLUDE_DIRECTORIES)
  list(FILTER dirs EXCLUDE REGEX "pthreads-w32")
  set_target_properties(${target} PROPERTIES INCLUDE_DIRECTORIES "${dirs}")
  target_include_directories(${target} BEFORE PRIVATE "${VC1_WEB_DIR}/include")
  target_compile_definitions(${target} PRIVATE
    BOOST_COMPILER_CONFIG="vc1_boost_config.hpp"
    BOOST_STDLIB_CONFIG="vc1_boost_config.hpp"
    BOOST_PLATFORM_CONFIG="vc1_boost_config.hpp")
  target_compile_options(${target} PRIVATE
    "SHELL:-include ${VC1_WEB_DIR}/include/vc1_web_crt.h")
  set_target_properties(${target} PROPERTIES SUFFIX ".js")
  target_link_options(${target} PRIVATE
    --profiling-funcs
    # The recovered code calls vtable slots and driver hooks through cast
    # function types, as the original does; wasm checks indirect call types.
    -sEMULATE_FUNCTION_POINTER_CASTS=1
    -sASYNCIFY=1
    -sASYNCIFY_STACK_SIZE=1048576
    -sSTACK_SIZE=2097152
    -sINITIAL_MEMORY=268435456
    -sALLOW_MEMORY_GROWTH=1
    -sENVIRONMENT=web
    -sINVOKE_RUN=0
    -sEXIT_RUNTIME=1
    -sFORCE_FILESYSTEM=1
    -lidbfs.js
    "-sEXPORTED_RUNTIME_METHODS=FS,IDBFS,ENV,callMain,addRunDependency,removeRunDependency")
  # The page plays the local game copy (git-ignored, never committed):
  # game.data / game.data.js preload VirtualCity.bar, the game's 7z archive
  # that the KD "bar" driver mounts from the working directory, at /game with
  # the cursors and icon of the original executable.
  set(game "${CMAKE_SOURCE_DIR}/Virtual-City_Win_EN_Full")
  if(NOT EXISTS "${game}/VirtualCity.bar")
    message(FATAL_ERROR "The web build packages ${game}/VirtualCity.bar; put the game copy there.")
  endif()
  # emsdk_env sets EMSDK_PYTHON, the interpreter the Emscripten tools want.
  if(DEFINED ENV{EMSDK_PYTHON})
    set(Python3_EXECUTABLE "$ENV{EMSDK_PYTHON}")
  else()
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
  endif()
  set(resources "${CMAKE_BINARY_DIR}/platform-resources")
  set(venv_python "${CMAKE_SOURCE_DIR}/.venv/Scripts/python.exe")
  set(packaged "${game}/VirtualCity.bar@/game/VirtualCity.bar")
  if(EXISTS "${game}/VirtualCity.Unwrapped.exe" AND EXISTS "${venv_python}")
    execute_process(COMMAND "${venv_python}" "${CMAKE_SOURCE_DIR}/tools/extract-sdl-resources.py"
      "${game}/VirtualCity.Unwrapped.exe" "${resources}" RESULT_VARIABLE extracted)
    if(extracted EQUAL 0)
      list(APPEND packaged "${resources}@/game/.vc1/platform-resources")
    endif()
  endif()
  set(data "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/game.data")
  add_custom_command(OUTPUT "${data}" "${data}.js"
    COMMAND "${Python3_EXECUTABLE}" "${EMSCRIPTEN_ROOT_PATH}/tools/file_packager.py" "${data}"
      --preload ${packaged} "--js-output=${data}.js" --use-preload-cache --no-node
      "--depfile=${CMAKE_BINARY_DIR}/game.data.d"
    DEPFILE "${CMAKE_BINARY_DIR}/game.data.d"
    COMMENT "Packaging the local game data for the page"
    VERBATIM)
  add_custom_target(vc1_web_data ALL DEPENDS "${data}" "${data}.js")
  # The page: index.html and loader.js start VirtualCity.js and the data.
  set(page)
  foreach(file index.html loader.js)
    add_custom_command(OUTPUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${file}"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${VC1_WEB_DIR}/shell/${file}"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${file}"
      DEPENDS "${VC1_WEB_DIR}/shell/${file}")
    list(APPEND page "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${file}")
  endforeach()
  add_custom_target(vc1_web_page ALL DEPENDS ${page})
endfunction()
