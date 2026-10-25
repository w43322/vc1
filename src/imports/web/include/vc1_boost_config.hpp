// Boost 1.41 has no configuration for clang, libc++ or Emscripten, and only
// the VC9 ones are vendored. The pool headers need none of their features;
// cmake/web.cmake names this file as the compiler, library and platform config.
#ifndef VC1_IMPORTS_WEB_BOOST_CONFIG_HPP
#define VC1_IMPORTS_WEB_BOOST_CONFIG_HPP
#define BOOST_HAS_LONG_LONG
#endif
