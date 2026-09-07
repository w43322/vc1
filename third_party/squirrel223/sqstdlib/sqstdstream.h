/*	see copyright notice in squirrel.h */
#ifndef _SQSTD_STREAM_H_
#define _SQSTD_STREAM_H_

SQInteger _stream_readblob(HSQUIRRELVM v);
SQInteger _stream_readline(HSQUIRRELVM v);
SQInteger _stream_readn(HSQUIRRELVM v);
SQInteger _stream_writeblob(HSQUIRRELVM v);
SQInteger _stream_writen(HSQUIRRELVM v);
SQInteger _stream_seek(HSQUIRRELVM v);
SQInteger _stream_tell(HSQUIRRELVM v);
SQInteger _stream_len(HSQUIRRELVM v);
SQInteger _stream_eos(HSQUIRRELVM v);
SQInteger _stream_flush(HSQUIRRELVM v);

#define _DECL_STREAM_FUNC(name,nparams,typecheck) {_SC(#name),_stream_##name,nparams,typecheck}
SQRESULT declare_stream(HSQUIRRELVM v,SQChar* name,SQUserPointer typetag,const SQChar* reg_name,SQRegFunction *methods,SQRegFunction *globals);

// FUNCTION: VC1 0x004c1620
// _stream_eos
// FUNCTION: VC1 0x004c16d0
// _stream_flush
// FUNCTION: VC1 0x004c1780
// _stream_len
// FUNCTION: VC1 0x004c1850
// _stream_tell
// FUNCTION: VC1 0x004c1920
// _stream_seek
// FUNCTION: VC1 0x004c1ab0
// _stream_writen
// FUNCTION: VC1 0x004c1d00
// _stream_readn
// FUNCTION: VC1 0x004c1f70
// _stream_writeblob
// FUNCTION: VC1 0x004c22c0
// _stream_readblob
// FUNCTION: VC1 0x004c20a0
// init_streamclass
// FUNCTION: VC1 0x004c2420
// declare_stream

#endif /*_SQSTD_STREAM_H_*/
