/*	see copyright notice in squirrel.h */
#ifndef _SQTABLE_H_
#define _SQTABLE_H_
/*
* The following code is based on Lua 4.0 (Copyright 1994-2002 Tecgraf, PUC-Rio.)
* http://www.lua.org/copyright.html#4
* http://www.lua.org/source/4.0.1/src_ltable.c.html
*/

#include "sqstring.h"


#define hashptr(p)  ((SQHash)(((SQInteger)p) >> 3))

inline SQHash HashObj(const SQObjectPtr &key)
{
	switch(type(key)) {
		case OT_STRING:		return _string(key)->_hash;
		case OT_FLOAT:		return (SQHash)((SQInteger)_float(key));
		case OT_BOOL: case OT_INTEGER:	return (SQHash)((SQInteger)_integer(key));
		default:			return hashptr(key._unVal.pRefCounted);
	}
}

// VTABLE: VC1 0x004d3a80
struct SQTable : public SQDelegable 
{
private:
	struct _HashNode
	{
		_HashNode() { next = NULL; }
		SQObjectPtr val;
		SQObjectPtr key;
		_HashNode *next;
	};
	_HashNode *_firstfree;
	_HashNode *_nodes;
	SQInteger _numofnodes;
	SQInteger _usednodes;
	
///////////////////////////
	void AllocNodes(SQInteger nSize);
	void Rehash(bool force);
	SQTable(SQSharedState *ss, SQInteger nInitialSize);
	void _ClearNodes();
public:
	static SQTable* Create(SQSharedState *ss,SQInteger nInitialSize)
	{
		SQTable *newtable = (SQTable*)SQ_MALLOC(sizeof(SQTable));
		new (newtable) SQTable(ss, nInitialSize);
		newtable->_delegate = NULL;
		return newtable;
	}
	void Finalize();
	SQTable *Clone();
	~SQTable()
	{
		SetDelegate(NULL);
		REMOVE_FROM_CHAIN(&_sharedstate->_gc_chain, this);
		for (SQInteger i = 0; i < _numofnodes; i++) _nodes[i].~_HashNode();
		SQ_FREE(_nodes, _numofnodes * sizeof(_HashNode));
	}
#ifndef NO_GARBAGE_COLLECTOR 
	void Mark(SQCollectable **chain);
#endif
	inline _HashNode *_Get(const SQObjectPtr &key,SQHash hash)
	{
		_HashNode *n = &_nodes[hash];
		do{
			if(_rawval(n->key) == _rawval(key) && type(n->key) == type(key)){
				return n;
			}
		}while((n = n->next));
		return NULL;
	}
	bool Get(const SQObjectPtr &key,SQObjectPtr &val);
	void Remove(const SQObjectPtr &key);
	bool Set(const SQObjectPtr &key, const SQObjectPtr &val);
	//returns true if a new slot has been created false if it was already present
	bool NewSlot(const SQObjectPtr &key,const SQObjectPtr &val);
	SQInteger Next(bool getweakrefs,const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval);
	
	SQInteger CountUsed(){ return _usednodes;}
	void Clear();
	void Release()
	{
		sq_delete(this, SQTable);
	}
	
};

// FUNCTION: VC1 0x0040d100
// SQTable::Create
// FUNCTION: VC1 0x00415330
// SQTable::~SQTable
// FUNCTION: VC1 0x00415450
// SQTable::Release
// FUNCTION: VC1 0x00415470
// SQTable::`scalar deleting destructor'
// FUNCTION: VC1 0x0048ad60
// SQTable::Mark
// FUNCTION: VC1 0x004b7120
// SQTable::_ClearNodes
// FUNCTION: VC1 0x004b71d0
// SQTable::Set
// FUNCTION: VC1 0x004b72b0
// SQTable::Next
// FUNCTION: VC1 0x004b73c0
// SQTable::NewSlot
// FUNCTION: VC1 0x004b75f0
// SQTable::Get
// FUNCTION: VC1 0x004b76f0
// SQTable::Finalize
// FUNCTION: VC1 0x004b7730
// SQTable::AllocNodes
// FUNCTION: VC1 0x004b77c0
// SQTable::SQTable
// FUNCTION: VC1 0x004b7810
// SQTable::Clone
// FUNCTION: VC1 0x004b7910
// SQTable::Rehash
// FUNCTION: VC1 0x004b7a60
// SQTable::Remove

#endif //_SQTABLE_H_
