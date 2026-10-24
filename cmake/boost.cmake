# Boost 1.41.0 (https://archives.boost.io/release/1.41.0/source/
# boost_1_41_0.tar.bz2, SHA-256 1ef94e6749eaf13318284b4f629be063544c7015b45e38113b975ac1945cc726),
# header-only: only the Boost.Pool headers and what they include under VC9,
# files unchanged. The game's size-class heap (src/memory/fixed_pool.*) is an
# array of boost::pool<default_user_allocator_malloc_free>; no Boost.Thread,
# so the pool mutex is the null one.
set(BOOST_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/third_party/boost_1_41_0")
set(BOOST_DEFINITIONS BOOST_DISABLE_THREADS BOOST_ALL_NO_LIB)
