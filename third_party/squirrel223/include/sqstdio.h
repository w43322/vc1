/*	see copyright notice in squirrel.h */
#ifndef _SQSTDIO_H_
#define _SQSTDIO_H_

#ifdef __cplusplus

#define SQSTD_STREAM_TYPE_TAG 0x80000000

// VTABLE: VC1 0x004d301c
struct SQStream {
	virtual ~SQStream() {}
	virtual SQInteger Read(void *buffer, SQInteger size) = 0;
	virtual SQInteger Write(void *buffer, SQInteger size) = 0;
	virtual SQInteger Flush() = 0;
	virtual SQInteger Tell() = 0;
	virtual SQInteger Len() = 0;
	virtual SQInteger Seek(SQInteger offset, SQInteger origin) = 0;
	virtual bool IsValid() = 0;
	virtual bool EOS() = 0;
};

extern "C" {
#endif

#define SQ_SEEK_CUR 0
#define SQ_SEEK_END 1
#define SQ_SEEK_SET 2

typedef void* SQFILE;

SQUIRREL_API SQFILE sqstd_fopen(const SQChar *,const SQChar *);
SQUIRREL_API SQInteger sqstd_fread(SQUserPointer, SQInteger, SQInteger, SQFILE);
SQUIRREL_API SQInteger sqstd_fwrite(const SQUserPointer, SQInteger, SQInteger, SQFILE);
SQUIRREL_API SQInteger sqstd_fseek(SQFILE , SQInteger , SQInteger);
SQUIRREL_API SQInteger sqstd_ftell(SQFILE);
SQUIRREL_API SQInteger sqstd_fflush(SQFILE);
SQUIRREL_API SQInteger sqstd_fclose(SQFILE);
SQUIRREL_API SQInteger sqstd_feof(SQFILE);

SQUIRREL_API SQRESULT sqstd_createfile(HSQUIRRELVM v, SQFILE file,SQBool own);
SQUIRREL_API SQRESULT sqstd_getfile(HSQUIRRELVM v, SQInteger idx, SQFILE *file);

//compiler helpers
SQUIRREL_API SQRESULT sqstd_loadfile(HSQUIRRELVM v,const SQChar *filename,SQBool printerror);
SQUIRREL_API SQRESULT sqstd_dofile(HSQUIRRELVM v,const SQChar *filename,SQBool retval,SQBool printerror);
SQUIRREL_API SQRESULT sqstd_writeclosuretofile(HSQUIRRELVM v,const SQChar *filename);

SQUIRREL_API SQRESULT sqstd_register_iolib(HSQUIRRELVM v);

#ifdef __cplusplus
} /*extern "C"*/
#endif

// FUNCTION: VC1 0x0040ed80
// SQStream::`scalar deleting destructor'
// FUNCTION: VC1 0x0040eda0
// SQFile::Write
// FUNCTION: VC1 0x0040ede0
// SQFile::Flush
// FUNCTION: VC1 0x0040ee10
// SQFile::Len
// FUNCTION: VC1 0x0040ee50
// SQFile::Seek
// FUNCTION: VC1 0x0040ee70
// SQFile::IsValid
// FUNCTION: VC1 0x0040ee80
// SQFile::EOS
// FUNCTION: VC1 0x0040eea0
// SQFile::`scalar deleting destructor'
// FUNCTION: VC1 0x0041d680
// SQFile::Read
// FUNCTION: VC1 0x0041d710
// SQFile::Tell
// FUNCTION: VC1 0x0048f240
// _file_releasehook
// FUNCTION: VC1 0x0048f310
// sqstd_fseek
// FUNCTION: VC1 0x0048f3f0
// file_write
// FUNCTION: VC1 0x0048f500
// _io_file_lexfeed_UTF8
// FUNCTION: VC1 0x0048f600
// _file__typeof
// FUNCTION: VC1 0x0048f620
// _file_constructor
// FUNCTION: VC1 0x0048f830
// _g_io_writeclosuretofile
// FUNCTION: VC1 0x0048f8b0
// sqstd_createfile
// FUNCTION: VC1 0x0048f990
// sqstd_register_iolib
// FUNCTION: VC1 0x0048fa50
// sqstd_loadfile
// FUNCTION: VC1 0x0048fbd0
// _g_io_loadfile
// FUNCTION: VC1 0x0048fc80
// sqstd_dofile
// FUNCTION: VC1 0x0048fd20
// _g_io_dofile

#endif /*_SQSTDIO_H_*/

