/* see copyright notice in squirrel.h */
// Host fork of Squirrel 2.2.3 sqstdlib/sqstdsystem.cpp (replaces it in the
// build). The game's systemlib runs on the KD layer: getenv through
// GetKdEnv, system opens its argument with ShellExecuteA, clock reads the
// performance counter in whole seconds, remove and rename translate
// "data/..." paths. Changes are marked VC1; every function keeps its name
// comment in sqstdsystem.h.
#include "platform/win32/runtime.h"

#include <shellapi.h>
#include <squirrel.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sqstdsystem.h>

static SQInteger _system_getenv(HSQUIRRELVM v)
{
	const SQChar *s;
	if(SQ_SUCCEEDED(sq_getstring(v,2,&s))){
		// VC1: the KD environment (KD_UDID falls back to COMPUTERNAME).
		sq_pushstring(v,vc1::GetKdEnv(s),-1);
		return 1;
	}
	return 0;
}


static SQInteger _system_system(HSQUIRRELVM v)
{
	const SQChar *s;
	if(SQ_SUCCEEDED(sq_getstring(v,2,&s))){
		// VC1: opens the document or URL; 0 on success, -1 on failure.
		// STRING: VC1 0x004d27e8
		HINSTANCE result = ShellExecuteA(0,"open",s,0,0,SW_SHOWNORMAL);
		sq_pushinteger(v,(int)result > 32 ? 0 : -1);
		return 1;
	}
	return sq_throwerror(v,_SC("wrong param"));
}


static SQInteger _system_clock(HSQUIRRELVM v)
{
	// VC1: the KD UST clock (nanoseconds) in whole seconds.
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	sq_pushinteger(v,(SQInteger)((unsigned __int64)((double)counter.QuadPart * vc1::g_nsPerTick)
		/ 1000000000));
	return 1;
}

static SQInteger _system_time(HSQUIRRELVM v)
{
	time_t t;
	time(&t);
	sq_pushinteger(v,*((SQInteger *)&t));
	return 1;
}

static SQInteger _system_remove(HSQUIRRELVM v)
{
	const SQChar *s;
	char path[260];
	sq_getstring(v,2,&s);
	// VC1: "data/..." resolves under the user data directory.
	if(remove(vc1::CopyExeDir(s,path))==-1)
		// STRING: VC1 0x004d2e80
		return sq_throwerror(v,_SC("remove() failed"));
	return 0;
}

static SQInteger _system_rename(HSQUIRRELVM v)
{
	const SQChar *oldn,*newn;
	sq_getstring(v,2,&oldn);
	sq_getstring(v,3,&newn);
	// VC1: both paths go through the "data/..." translation.
	if(vc1::KdRename(oldn,newn)==-1)
		// STRING: VC1 0x004d2e90
		return sq_throwerror(v,_SC("rename() failed"));
	return 0;
}

static void _set_integer_slot(HSQUIRRELVM v,const SQChar *name,SQInteger val)
{
	sq_pushstring(v,name,-1);
	sq_pushinteger(v,val);
	sq_rawset(v,-3);
}

static SQInteger _system_date(HSQUIRRELVM v)
{
	time_t t;
	SQInteger it;
	SQInteger format = 'l';
	if(sq_gettop(v) > 1) {
		sq_getinteger(v,2,&it);
		t = it;
		if(sq_gettop(v) > 2) {
			sq_getinteger(v,3,(SQInteger*)&format);
		}
	}
	else {
		time(&t);
	}
	// VC1: no "crt api failure" check; the game copies the result directly.
	tm date = format == 'u' ? *gmtime(&t) : *localtime(&t);
	sq_newtable(v);
	_set_integer_slot(v, _SC("sec"), date.tm_sec);
    _set_integer_slot(v, _SC("min"), date.tm_min);
    _set_integer_slot(v, _SC("hour"), date.tm_hour);
    _set_integer_slot(v, _SC("day"), date.tm_mday);
    _set_integer_slot(v, _SC("month"), date.tm_mon);
    _set_integer_slot(v, _SC("year"), date.tm_year+1900);
    _set_integer_slot(v, _SC("wday"), date.tm_wday);
    _set_integer_slot(v, _SC("yday"), date.tm_yday);
	return 1;
}



#define _DECL_FUNC(name,nparams,pmask) {_SC(#name),_system_##name,nparams,pmask}
static SQRegFunction systemlib_funcs[]={
	_DECL_FUNC(getenv,2,_SC(".s")),
	_DECL_FUNC(system,2,_SC(".s")),
	_DECL_FUNC(clock,1,NULL),
	_DECL_FUNC(time,1,NULL),
	_DECL_FUNC(date,-1,_SC(".nn")),
	_DECL_FUNC(remove,2,_SC(".s")),
	_DECL_FUNC(rename,3,_SC(".ss")),
	{0,0}
};


SQInteger sqstd_register_systemlib(HSQUIRRELVM v)
{
	SQInteger i=0;
	while(systemlib_funcs[i].name!=0)
	{
		sq_pushstring(v,systemlib_funcs[i].name,-1);
		sq_newclosure(v,systemlib_funcs[i].f,0);
		sq_setparamscheck(v,systemlib_funcs[i].nparamscheck,systemlib_funcs[i].typemask);
		sq_setnativeclosurename(v,-1,systemlib_funcs[i].name);
		sq_createslot(v,-3);
		i++;
	}
	return 1;
}
