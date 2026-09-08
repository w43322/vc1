/*	see copyright notice in squirrel.h */
#ifndef _SQSTRING_H_
#define _SQSTRING_H_

inline SQHash _hashstr (const SQChar *s, size_t l)
{
		SQHash h = (SQHash)l;  /* seed */
		size_t step = (l>>5)|1;  /* if string is too long, don't hash all its chars */
		for (; l>=step; l-=step)
			h = h ^ ((h<<5)+(h>>2)+(unsigned short)*(s++));
		return h;
}

// VTABLE: VC1 0x004d3734
struct SQString : public SQRefCounted
{
	SQString(){}
	~SQString(){}
public:
	static SQString *Create(SQSharedState *ss, const SQChar *, SQInteger len = -1 );
	SQInteger Next(const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval);
	void Release();
	SQSharedState *_sharedstate;
	SQString *_next; //chain for the string table
	SQInteger _len;
	SQHash _hash;
	SQChar _val[1];
};

// SYNTHETIC: VC1 0x0040f730 FOLDED
// SQString::`scalar deleting destructor'
// FUNCTION: VC1 0x0040f840
// _hashstr
// FUNCTION: VC1 0x0048bc60
// SQString::Release



#endif //_SQSTRING_H_
