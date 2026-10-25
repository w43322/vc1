/*	see copyright notice in squirrel.h */
#ifndef _SQCLASS_H_
#define _SQCLASS_H_

struct SQInstance;

struct SQClassMember {
	SQClassMember(){}
	SQClassMember(const SQClassMember &o) {
		val = o.val;
		attrs = o.attrs;
	}
	SQObjectPtr val;
	SQObjectPtr attrs;
};

typedef sqvector<SQClassMember> SQClassMemberVec;

#define MEMBER_TYPE_METHOD 0x01000000
#define MEMBER_TYPE_FIELD 0x02000000

#define _ismethod(o) (_integer(o)&MEMBER_TYPE_METHOD)
#define _isfield(o) (_integer(o)&MEMBER_TYPE_FIELD)
#define _make_method_idx(i) ((SQInteger)(MEMBER_TYPE_METHOD|i))
#define _make_field_idx(i) ((SQInteger)(MEMBER_TYPE_FIELD|i))
#define _member_type(o) (_integer(o)&0xFF000000)
#define _member_idx(o) (_integer(o)&0x00FFFFFF)

// VTABLE: VC1 0x004d373c
struct SQClass : public CHAINABLE_OBJ
{
	SQClass(SQSharedState *ss,SQClass *base);
public:
	static SQClass* Create(SQSharedState *ss,SQClass *base) {
		SQClass *newclass = (SQClass *)SQ_MALLOC(sizeof(SQClass));
		new (newclass) SQClass(ss, base);
		return newclass;
	}
	~SQClass();
	bool NewSlot(SQSharedState *ss, const SQObjectPtr &key,const SQObjectPtr &val,bool bstatic);
	bool Get(const SQObjectPtr &key,SQObjectPtr &val) {
		if(_members->Get(key,val)) {
			if(_isfield(val)) {
				SQObjectPtr &o = _defaultvalues[_member_idx(val)].val;
				val = _realval(o);
			}
			else {
				val = _methods[_member_idx(val)].val;
			}
			return true;
		}
		return false;
	}
	bool SetAttributes(const SQObjectPtr &key,const SQObjectPtr &val);
	bool GetAttributes(const SQObjectPtr &key,SQObjectPtr &outval);
	void Lock() { _locked = true; if(_base) _base->Lock(); }
	void Release() { 
		if (_hook) { _hook(_typetag,0);}
		sq_delete(this, SQClass);	
	}
	void Finalize();
#ifndef NO_GARBAGE_COLLECTOR
	void Mark(SQCollectable ** );
#endif
	SQInteger Next(const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval);
	SQInstance *CreateInstance();
	SQTable *_members;
	SQClass *_base;
	SQClassMemberVec _defaultvalues;
	SQClassMemberVec _methods;
	SQObjectPtrVec _metamethods;
	SQObjectPtr _attributes;
	SQUserPointer _typetag;
	SQRELEASEHOOK _hook;
	bool _locked;
	SQInteger _udsize;
};

#define calcinstancesize(_theclass_) \
	(_theclass_->_udsize + sizeof(SQInstance) + (sizeof(SQObjectPtr)*(_theclass_->_defaultvalues.size()>0?_theclass_->_defaultvalues.size()-1:0)))

// VTABLE: VC1 0x004d374c
struct SQInstance : public SQDelegable 
{
	void Init(SQSharedState *ss);
	SQInstance(SQSharedState *ss, SQClass *c, SQInteger memsize);
	SQInstance(SQSharedState *ss, SQInstance *c, SQInteger memsize);
public:
	static SQInstance* Create(SQSharedState *ss,SQClass *theclass) {
		
		SQInteger size = calcinstancesize(theclass);
		SQInstance *newinst = (SQInstance *)SQ_MALLOC(size);
		new (newinst) SQInstance(ss, theclass,size);
		if(theclass->_udsize) {
			newinst->_userpointer = ((unsigned char *)newinst) + (size - theclass->_udsize);
		}
		return newinst;
	}
	SQInstance *Clone(SQSharedState *ss)
	{
		SQInteger size = calcinstancesize(_class);
		SQInstance *newinst = (SQInstance *)SQ_MALLOC(size);
		new (newinst) SQInstance(ss, this,size);
		if(_class->_udsize) {
			newinst->_userpointer = ((unsigned char *)newinst) + (size - _class->_udsize);
		}
		return newinst;
	}
	~SQInstance();
	bool Get(const SQObjectPtr &key,SQObjectPtr &val)  {
		if(_class->_members->Get(key,val)) {
			if(_isfield(val)) {
				SQObjectPtr &o = _values[_member_idx(val)];
				val = _realval(o);
			}
			else {
				val = _class->_methods[_member_idx(val)].val;
			}
			return true;
		}
		return false;
	}
	bool Set(const SQObjectPtr &key,const SQObjectPtr &val) {
		SQObjectPtr idx;
		if(_class->_members->Get(key,idx) && _isfield(idx)) {
            _values[_member_idx(idx)] = val;
			return true;
		}
		return false;
	}
	void Release() {
		_uiRef++;
		if (_hook) { _hook(_userpointer,0);}
		_uiRef--;
		if(_uiRef > 0) return;
		SQInteger size = _memsize;
		this->~SQInstance();
		SQ_FREE(this, size);
	}
	void Finalize();
#ifndef NO_GARBAGE_COLLECTOR 
	void Mark(SQCollectable ** );
#endif
	bool InstanceOf(SQClass *trg);
	bool GetMetaMethod(SQVM *v,SQMetaMethod mm,SQObjectPtr &res);

	SQClass *_class;
	SQUserPointer _userpointer;
	SQRELEASEHOOK _hook;
	SQInteger _memsize;
	SQObjectPtr _values[1];
};

// FUNCTION: VC1 0x0040db90
// ?Get@SQClass@@QAE_NABUSQObjectPtr@@AAU2@@Z
// FUNCTION: VC1 0x0040dc30
// ?Get@SQInstance@@QAE_NABUSQObjectPtr@@AAU2@@Z
// FUNCTION: VC1 0x0040dd10
// ?Set@SQInstance@@QAE_NABUSQObjectPtr@@0@Z
// FUNCTION: VC1 0x0040ea70
// SQInstance::Clone
// FUNCTION: VC1 0x0040f920
// ??0SQClassMember@@QAE@ABU0@@Z
// FUNCTION: VC1 0x0040f9a0
// ?Lock@SQClass@@QAEXXZ
// FUNCTION: VC1 0x0040f9c0
// SQClass::Release
// FUNCTION: VC1 0x0040fa00
// SQInstance::Create
// FUNCTION: VC1 0x0040fa80
// SQInstance::Release
// SYNTHETIC: VC1 0x0040fae0
// SQClass::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0040fb00
// ??1SQClassMember@@QAE@XZ
// SYNTHETIC: VC1 0x0040fb40
// SQInstance::`scalar deleting destructor'
// FUNCTION: VC1 0x00487540
// SQInstance::InstanceOf
// FUNCTION: VC1 0x00487560
// SQInstance::GetMetaMethod
// FUNCTION: VC1 0x004875c0
// SQInstance::Finalize
// FUNCTION: VC1 0x00487680
// SQClass::GetAttributes
// FUNCTION: VC1 0x00487760
// SQClass::SetAttributes
// FUNCTION: VC1 0x00487880
// SQClass::Next
// FUNCTION: VC1 0x00487980
// SQInstance::~SQInstance
// FUNCTION: VC1 0x00487a10
// ??0SQInstance@@QAE@PAUSQSharedState@@PAU0@H@Z
// FUNCTION: VC1 0x00487ab0
// ??0SQInstance@@QAE@PAUSQSharedState@@PAUSQClass@@H@Z
// FUNCTION: VC1 0x00487b80
// SQClass::NewSlot
// FUNCTION: VC1 0x00487da0
// SQClass::Finalize
// FUNCTION: VC1 0x00487ef0
// SQClass::SQClass
// FUNCTION: VC1 0x00488050
// SQClass::~SQClass
// FUNCTION: VC1 0x0048b480
// SQInstance::Mark
// FUNCTION: VC1 0x0048b560
// SQClass::Mark

#endif //_SQCLASS_H_
