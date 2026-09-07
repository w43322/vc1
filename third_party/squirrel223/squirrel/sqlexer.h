/*	see copyright notice in squirrel.h */
#ifndef _SQLEXER_H_
#define _SQLEXER_H_

#ifdef SQUNICODE
typedef SQChar LexChar;
#else
typedef	unsigned char LexChar;
#endif

struct SQLexer
{
	SQLexer();
	~SQLexer();
	void Init(SQSharedState *ss,SQLEXREADFUNC rg,SQUserPointer up,CompilerErrorFunc efunc,void *ed);
	void Error(const SQChar *err);
	SQInteger Lex();
	const SQChar *Tok2Str(SQInteger tok);
private:
	SQInteger GetIDType(SQChar *s);
	SQInteger ReadString(SQInteger ndelim,bool verbatim);
	SQInteger ReadNumber();
	void LexBlockComment();
	SQInteger ReadID();
	void Next();
	SQInteger _curtoken;
	SQTable *_keywords;
public:
	SQInteger _prevtoken;
	SQInteger _currentline;
	SQInteger _lasttokenline;
	SQInteger _currentcolumn;
	const SQChar *_svalue;
	SQInteger _nvalue;
	SQFloat _fvalue;
	SQLEXREADFUNC _readf;
	SQUserPointer _up;
	LexChar _currdata;
	SQSharedState *_sharedstate;
	sqvector<SQChar> _longstr;
	CompilerErrorFunc _errfunc;
	void *_errtarget;
};

// FUNCTION: VC1 0x0047ef30
// SQLexer::Next
// FUNCTION: VC1 0x0047ef70
// SQLexer::LexBlockComment
// FUNCTION: VC1 0x0047f0a0
// SQLexer::Tok2Str
// FUNCTION: VC1 0x0047f220
// SQLexer::ReadNumber
// FUNCTION: VC1 0x0047f720
// SQLexer::ReadString
// FUNCTION: VC1 0x0047fcf0
// SQLexer::GetIDType
// FUNCTION: VC1 0x0047fdb0
// SQLexer::Init
// FUNCTION: VC1 0x004811d0
// SQLexer::ReadID
// FUNCTION: VC1 0x004812d0
// SQLexer::Lex

#endif
