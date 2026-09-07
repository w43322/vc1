/*	see copyright notice in squirrel.h */
#ifndef _SQCLOSURE_H_
#define _SQCLOSURE_H_

struct SQFunctionProto;

// VTABLE: VC1 0x004d2794
struct SQClosure : public CHAINABLE_OBJ
{
private:
	SQClosure(SQSharedState *ss,SQFunctionProto *func){_function=func; INIT_CHAIN();ADD_TO_CHAIN(&_ss(this)->_gc_chain,this);}
public:
	static SQClosure *Create(SQSharedState *ss,SQFunctionProto *func){
		SQClosure *nc=(SQClosure*)SQ_MALLOC(sizeof(SQClosure));
		new (nc) SQClosure(ss,func);
		return nc;
	}
	void Release(){
		sq_delete(this,SQClosure);
	}
	SQClosure *Clone()
	{
		SQClosure * ret = SQClosure::Create(_opt_ss(this),_funcproto(_function));
		ret->_env = _env;
		ret->_outervalues.copy(_outervalues);
		ret->_defaultparams.copy(_defaultparams);
		return ret;
	}
	~SQClosure()
	{
		REMOVE_FROM_CHAIN(&_ss(this)->_gc_chain,this);
	}
	bool Save(SQVM *v,SQUserPointer up,SQWRITEFUNC write);
	static bool Load(SQVM *v,SQUserPointer up,SQREADFUNC read,SQObjectPtr &ret);
#ifndef NO_GARBAGE_COLLECTOR
	void Mark(SQCollectable **chain);
	void Finalize(){_outervalues.resize(0); }
#endif
	SQObjectPtr _env;
	SQObjectPtr _function;
	SQObjectPtrVec _outervalues;
	SQObjectPtrVec _defaultparams;
};
//////////////////////////////////////////////
// VTABLE: VC1 0x004d2d60
struct SQGenerator : public CHAINABLE_OBJ 
{
	enum SQGeneratorState{eRunning,eSuspended,eDead};
private:
	SQGenerator(SQSharedState *ss,SQClosure *closure){_closure=closure;_state=eRunning;_ci._generator=NULL;INIT_CHAIN();ADD_TO_CHAIN(&_ss(this)->_gc_chain,this);}
public:
	static SQGenerator *Create(SQSharedState *ss,SQClosure *closure){
		SQGenerator *nc=(SQGenerator*)SQ_MALLOC(sizeof(SQGenerator));
		new (nc) SQGenerator(ss,closure);
		return nc;
	}
	~SQGenerator()
	{
		REMOVE_FROM_CHAIN(&_ss(this)->_gc_chain,this);
	}
    void Kill(){
		_state=eDead;
		_stack.resize(0);
		_closure=_null_;}
	void Release(){
		sq_delete(this,SQGenerator);
	}
	bool Yield(SQVM *v);
	bool Resume(SQVM *v,SQInteger target);
#ifndef NO_GARBAGE_COLLECTOR
	void Mark(SQCollectable **chain);
	void Finalize(){_stack.resize(0);_closure=_null_;}
#endif
	SQObjectPtr _closure;
	SQObjectPtrVec _stack;
	SQObjectPtrVec _vargsstack;
	SQVM::CallInfo _ci;
	ExceptionsTraps _etraps;
	SQGeneratorState _state;
};

// VTABLE: VC1 0x004d27a4
struct SQNativeClosure : public CHAINABLE_OBJ
{
private:
	SQNativeClosure(SQSharedState *ss,SQFUNCTION func){_function=func;INIT_CHAIN();ADD_TO_CHAIN(&_ss(this)->_gc_chain,this);	}
public:
	static SQNativeClosure *Create(SQSharedState *ss,SQFUNCTION func)
	{
		SQNativeClosure *nc=(SQNativeClosure*)SQ_MALLOC(sizeof(SQNativeClosure));
		new (nc) SQNativeClosure(ss,func);
		return nc;
	}
	SQNativeClosure *Clone()
	{
		SQNativeClosure * ret = SQNativeClosure::Create(_opt_ss(this),_function);
		ret->_env = _env;
		ret->_name = _name;
		ret->_outervalues.copy(_outervalues);
		ret->_typecheck.copy(_typecheck);
		ret->_nparamscheck = _nparamscheck;
		return ret;
	}
	~SQNativeClosure()
	{
		REMOVE_FROM_CHAIN(&_ss(this)->_gc_chain,this);
	}
	void Release(){
		sq_delete(this,SQNativeClosure);
	}
#ifndef NO_GARBAGE_COLLECTOR
	void Mark(SQCollectable **chain);
	void Finalize(){_outervalues.resize(0);}
#endif
	SQInteger _nparamscheck;
	SQIntVec _typecheck;
	SQObjectPtrVec _outervalues;
	SQObjectPtr _env;
	SQFUNCTION _function;
	SQObjectPtr _name;
};

// FUNCTION: VC1 0x0040d4a0
// SQClosure::SQClosure
// FUNCTION: VC1 0x0040d540
// SQClosure::Create
// FUNCTION: VC1 0x0040d580
// SQClosure::Release
// FUNCTION: VC1 0x0040d5a0
// SQClosure::Clone
// FUNCTION: VC1 0x0040d640
// SQClosure::~SQClosure
// FUNCTION: VC1 0x0040d700
// SQClosure::`scalar deleting destructor'
// FUNCTION: VC1 0x0040d720
// SQNativeClosure::SQNativeClosure
// FUNCTION: VC1 0x0040d780
// SQNativeClosure::Clone
// FUNCTION: VC1 0x0040d860
// SQNativeClosure::~SQNativeClosure
// FUNCTION: VC1 0x0040d950
// SQNativeClosure::Release
// FUNCTION: VC1 0x0040d970 FOLDED
// SQClosure::Finalize
// FUNCTION: VC1 0x0040d970 FOLDED
// SQNativeClosure::Finalize
// FUNCTION: VC1 0x0040d9a0
// SQNativeClosure::`scalar deleting destructor'
// FUNCTION: VC1 0x0040e550
// SQGenerator::SQGenerator
// FUNCTION: VC1 0x0040e610
// SQGenerator::Create
// FUNCTION: VC1 0x0040e650
// SQGenerator::~SQGenerator
// FUNCTION: VC1 0x0040e7c0
// SQGenerator::Release
// FUNCTION: VC1 0x0040e850
// SQGenerator::`scalar deleting destructor'
// FUNCTION: VC1 0x0048b0e0
// SQNativeClosure::Mark
// FUNCTION: VC1 0x0048b1b0
// SQClosure::Mark
// FUNCTION: VC1 0x0048b2f0
// SQGenerator::Mark
// FUNCTION: VC1 0x0048d270
// SQClosure::Load
// FUNCTION: VC1 0x0048d370
// SQClosure::Save

#endif //_SQCLOSURE_H_
