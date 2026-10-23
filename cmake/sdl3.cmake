# SDL is only resolved for the modern, opt-in transition build.
# Prefer an exact installed SDK, otherwise fetch the official pinned release.
find_package(SDL3 3.4.16 EXACT CONFIG QUIET)
if(NOT TARGET SDL3::SDL3)
  if(CMAKE_VERSION VERSION_LESS 3.16)
    message(FATAL_ERROR "Fetching SDL3 requires CMake 3.16 or newer.")
  endif()
  include(FetchContent)
  if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
  endif()
  set(SDL_SHARED ON CACHE BOOL "" FORCE)
  set(SDL_STATIC OFF CACHE BOOL "" FORCE)
  set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
  set(SDL_TESTS OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(SDL3
    URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.16/SDL3-3.4.16.tar.gz
    URL_HASH SHA256=7322236cd12090c3eb40b9728be4d49c76f66ad17d04369584d4ecad5cf77c68)
  FetchContent_MakeAvailable(SDL3)
endif()
