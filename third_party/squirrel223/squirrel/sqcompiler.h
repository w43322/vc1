/*	see copyright notice in squirrel.h */
#ifndef _SQCOMPILER_H_
#define _SQCOMPILER_H_

struct SQVM;

#define	TK_IDENTIFIER	258
#define	TK_STRING_LITERAL	259
#define	TK_INTEGER	260
#define	TK_FLOAT	261
#define	TK_DELEGATE	262
#define	TK_DELETE	263
#define	TK_EQ	264
#define	TK_NE	265
#define	TK_LE	266
#define	TK_GE	267
#define	TK_SWITCH	268
#define	TK_ARROW	269
#define	TK_AND	270
#define	TK_OR	271
#define	TK_IF	272
#define	TK_ELSE	273
#define	TK_WHILE	274
#define	TK_BREAK	275
#define	TK_FOR	276
#define	TK_DO	277
#define	TK_NULL	278
#define	TK_FOREACH	279
#define	TK_IN	280
#define	TK_NEWSLOT	281
#define	TK_MODULO	282
#define	TK_LOCAL	283
#define	TK_CLONE	284
#define	TK_FUNCTION	285
#define	TK_RETURN	286
#define	TK_TYPEOF	287
#define	TK_UMINUS	288
#define	TK_PLUSEQ	289
#define	TK_MINUSEQ	290
#define	TK_CONTINUE	291
#define TK_YIELD 292
#define TK_TRY 293
#define TK_CATCH 294
#define TK_THROW 295
#define TK_SHIFTL 296
#define TK_SHIFTR 297
#define TK_RESUME 298
#define TK_DOUBLE_COLON 299
#define TK_CASE 300
#define TK_DEFAULT 301
#define TK_THIS 302
#define TK_PLUSPLUS 303
#define TK_MINUSMINUS 304
#define TK_PARENT 305
#define TK_USHIFTR 306
#define TK_CLASS 307
#define TK_EXTENDS 308
#define TK_CONSTRUCTOR 310
#define TK_INSTANCEOF 311
#define TK_VARPARAMS 312
#define TK_VARGC 313
#define TK_VARGV 314
#define TK_TRUE 315
#define TK_FALSE 316
#define TK_MULEQ 317
#define TK_DIVEQ 318
#define TK_MODEQ 319
#define TK_ATTR_OPEN 320
#define TK_ATTR_CLOSE 321
#define TK_STATIC 322
#define TK_ENUM 323
#define TK_CONST 324


typedef void(*CompilerErrorFunc)(void *ud, const SQChar *s);
bool Compile(SQVM *vm, SQLEXREADFUNC rg, SQUserPointer up, const SQChar *sourcename, SQObjectPtr &out, bool raiseerror, bool lineinfo);

// FUNCTION: VC1 0x0040fe40
// ??0SQCompiler@@QAE@PAUSQVM@@P6AHPAX@Z1PBD_N4@Z
// FUNCTION: VC1 0x0040ff00
// SQCompiler::ThrowError
// FUNCTION: VC1 0x0040ff10
// SQCompiler::Error
// FUNCTION: VC1 0x0040ff40
// SQCompiler::IsEndOfStatement
// FUNCTION: VC1 0x0040ffe0
// SQCompiler::Compile
// FUNCTION: VC1 0x00410330
// SQCompiler::Statements
// FUNCTION: VC1 0x00410390
// SQCompiler::Statement
// FUNCTION: VC1 0x00410ae0
// SQCompiler::EmitDerefOp
// FUNCTION: VC1 0x00410b70
// SQCompiler::Emit2ArgsOP
// FUNCTION: VC1 0x00410bf0
// SQCompiler::EmitCompoundArith
// FUNCTION: VC1 0x00410d20
// SQCompiler::CommaExpr
// FUNCTION: VC1 0x00410d70
// SQCompiler::Expression
// FUNCTION: VC1 0x00411080
// SQCompiler::BIN_EXP
// FUNCTION: VC1 0x00411110
// SQCompiler::LogicalOrExp
// FUNCTION: VC1 0x004116a0
// SQCompiler::PrefixedExpr
// FUNCTION: VC1 0x00411ae0
// SQCompiler::Factor
// FUNCTION: VC1 0x00412520
// SQCompiler::UnaryOP
// FUNCTION: VC1 0x004125a0
// SQCompiler::NeedGet
// FUNCTION: VC1 0x00412630
// SQCompiler::FunctionCallArgs
// FUNCTION: VC1 0x00412760
// SQCompiler::ParseTableOrClass
// FUNCTION: VC1 0x00412c60
// SQCompiler::LocalDeclStatement
// FUNCTION: VC1 0x00412e40
// SQCompiler::IfStatement
// FUNCTION: VC1 0x00413010
// SQCompiler::WhileStatement
// FUNCTION: VC1 0x00413190
// SQCompiler::DoWhileStatement
// FUNCTION: VC1 0x00413310
// SQCompiler::ForStatement
// FUNCTION: VC1 0x00413710
// SQCompiler::ForEachStatement
// FUNCTION: VC1 0x00413af0
// SQCompiler::SwitchStatement
// FUNCTION: VC1 0x00413d70
// SQCompiler::FunctionStatement
// FUNCTION: VC1 0x00414080
// SQCompiler::ClassStatement
// FUNCTION: VC1 0x00414120
// SQCompiler::ExpectScalar
// FUNCTION: VC1 0x004142d0
// SQCompiler::EnumStatement
// FUNCTION: VC1 0x004145a0
// SQCompiler::TryCatchStatement
// FUNCTION: VC1 0x00414820
// SQCompiler::FunctionExp
// FUNCTION: VC1 0x004148c0
// SQCompiler::ClassExp
// FUNCTION: VC1 0x00414a10
// SQCompiler::DelegateExpr
// FUNCTION: VC1 0x00414ac0
// SQCompiler::DeleteExpr
// FUNCTION: VC1 0x00414b60
// SQCompiler::PrefixIncDec
// FUNCTION: VC1 0x00414c60
// SQCompiler::CreateFunction
// FUNCTION: VC1 0x00415190
// SQCompiler::ResolveBreaks
// FUNCTION: VC1 0x004151e0
// SQCompiler::ResolveContinues
// SYNTHETIC: VC1 0x00415230
// ??1SQCompiler@@QAE@XZ

#endif //_SQCOMPILER_H_
