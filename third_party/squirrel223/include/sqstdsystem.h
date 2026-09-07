/*	see copyright notice in squirrel.h */
#ifndef _SQSTD_SYSTEMLIB_H_
#define _SQSTD_SYSTEMLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

SQUIRREL_API SQInteger sqstd_register_systemlib(HSQUIRRELVM v);

#ifdef __cplusplus
} /*extern "C"*/
#endif

// FUNCTION: VC1 0x004baf50
// _system_time
// FUNCTION: VC1 0x004bafb0
// _system_clock
// FUNCTION: VC1 0x004bb070
// _system_rename
// FUNCTION: VC1 0x004bb160
// _system_remove
// FUNCTION: VC1 0x004bb210
// _system_system
// FUNCTION: VC1 0x004bb2e0
// _system_getenv
// FUNCTION: VC1 0x004bb370
// _system_date
// FUNCTION: VC1 0x004bb6c0
// sqstd_register_systemlib

#endif /* _SQSTD_SYSTEMLIB_H_ */
