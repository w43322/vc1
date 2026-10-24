#ifndef VC1_BOOST_VC1_H
#define VC1_BOOST_VC1_H

// Boost 1.41.0 mappings for VirtualCity.Unwrapped.exe. The headers in this
// directory are unchanged copies from boost_1_41_0.tar.bz2 (cmake/boost.cmake
// has the archive and its SHA-256): Boost.Pool and the headers VC9 includes
// for it. LICENSE_1_0.txt is the Boost Software License.
//
// Version: boost/pool/*.hpp are identical from 1.35.0 to 1.45.0. 1.34.1 has
// no pool::start_size (the original restores next_size from +0x14 in
// purge_memory), 1.46.0 adds max_size (the original pool is 0x18 bytes), and
// the exe was linked on 2009-12-21, after 1.41.0 and before 1.42.0.
//
// The game's size-class heap (src/memory/fixed_pool.*) is 33
// boost::pool<default_user_allocator_malloc_free>; pool::malloc,
// pool::free and simple_segregated_storage are inlined at every caller,
// and ~pool/purge_memory into DestroySizeClassPools (0x004b7c60).

// EBX=this. segregate inlined; gcd as a do-while dividing by 4 first.
// FUNCTION: VC1 0x0040f1a0
// ?malloc_need_resize@?$pool@Udefault_user_allocator_malloc_free@boost@@@boost@@AAEPAXXZ

#endif
