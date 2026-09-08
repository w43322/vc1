/*	see copyright notice in squirrel.h */
#ifndef _SQUSERDATA_H_
#define _SQUSERDATA_H_

// VTABLE: VC1 0x004d27b4
struct SQUserData : SQDelegable
{
	SQUserData(SQSharedState *ss){ _delegate = 0; _hook = NULL; INIT_CHAIN(); ADD_TO_CHAIN(&_ss(this)->_gc_chain, this); }
	~SQUserData()
	{
		REMOVE_FROM_CHAIN(&_ss(this)->_gc_chain, this);
		SetDelegate(NULL);
	}
	static SQUserData* Create(SQSharedState *ss, SQInteger size)
	{
		SQUserData* ud = (SQUserData*)SQ_MALLOC(sizeof(SQUserData)+(size-1));
		new (ud) SQUserData(ss);
		ud->_size = size;
		ud->_typetag = 0;
		return ud;
	}
#ifndef NO_GARBAGE_COLLECTOR
	void Mark(SQCollectable **chain);
	void Finalize(){SetDelegate(NULL);}
#endif
	void Release() {
		if (_hook) _hook(_val,_size);
		SQInteger tsize = _size - 1;
		this->~SQUserData();
		SQ_FREE(this, sizeof(SQUserData) + tsize);
	}
		
	SQInteger _size;
	SQRELEASEHOOK _hook;
	SQUserPointer _typetag;
	SQChar _val[1];
};

// FUNCTION: VC1 0x0040d9c0
// SQUserData::~SQUserData
// FUNCTION: VC1 0x0040da40
// SQUserData::Create
// FUNCTION: VC1 0x0040dad0
// SQUserData::Finalize
// FUNCTION: VC1 0x0040db10
// SQUserData::Release
// SYNTHETIC: VC1 0x0040db70
// SQUserData::`scalar deleting destructor'
// FUNCTION: VC1 0x0048acf0
// SQUserData::Mark

#endif //_SQUSERDATA_H_
