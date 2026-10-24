#ifndef _SQUIRREL_OBJECT_H_
#define _SQUIRREL_OBJECT_H_

// VTABLE: VC1 0x004cfb98
class SquirrelObject
{
  friend class SquirrelVM;
  
public:
  SquirrelObject();
  virtual ~SquirrelObject();
  SquirrelObject(const SquirrelObject & o);
  SquirrelObject(HSQOBJECT o);
  
#if 1
  template <typename _ty>
  SquirrelObject(const _ty & val) { sq_resetobject(&_o); Set((_ty &)val); } // Cast away const to avoid compiler SqPlus::Push() match issue.
  template <typename _ty>
  SquirrelObject(_ty & val) { sq_resetobject(&_o); Set(val); }
  template <typename _ty>
  SquirrelObject(_ty * val) { sq_resetobject(&_o); SetByValue(val); } // Set() would also be OK here. SetByValue() to save potential compiler overhead.
#endif

  SquirrelObject & operator = (HSQOBJECT ho);
  SquirrelObject & operator = (const SquirrelObject &o);
  SquirrelObject & operator = (int n);
  SquirrelObject & operator = (HSQUIRRELVM v);
  
  operator HSQOBJECT& (){ return _o; } 
  bool operator ==(const SquirrelObject& o);
  bool CompareUserPointer(const SquirrelObject& o);
  
  void AttachToStackObject(int idx);
  void Reset(void); // Release (any) reference and reset _o.
  SquirrelObject Clone();
  BOOL SetValue(const SquirrelObject &key,const SquirrelObject &val);
  
  BOOL SetValue(SQInteger key,const SquirrelObject &val);
  BOOL SetValue(INT key,bool b); // Compiler treats SQBool as INT.
  BOOL SetValue(INT key,INT n);
  BOOL SetValue(INT key,FLOAT f);
  BOOL SetValue(INT key,const SQChar *s);

  BOOL SetValue(const SQChar *key,const SquirrelObject &val);
  BOOL SetValue(const SQChar *key,bool b);
  BOOL SetValue(const SQChar *key,INT n);
  BOOL SetValue(const SQChar *key,FLOAT f);
  BOOL SetValue(const SQChar *key,const SQChar *s);

  BOOL SetUserPointer(const SQChar * key,SQUserPointer up);
  SQUserPointer GetUserPointer(const SQChar * key);
  BOOL SetUserPointer(INT key,SQUserPointer up);
  SQUserPointer GetUserPointer(INT key);

  BOOL NewUserData(const SQChar * key,INT size,SQUserPointer * typetag=0);
  BOOL GetUserData(const SQChar * key,SQUserPointer * data,SQUserPointer * typetag=0);
  BOOL RawGetUserData(const SQChar * key,SQUserPointer * data,SQUserPointer * typetag=0);

  // === BEGIN Arrays ===

  BOOL ArrayResize(INT newSize);
  BOOL ArrayExtend(INT amount);
  BOOL ArrayReverse(void);
  SquirrelObject ArrayPop(SQBool returnPoppedVal=SQTrue);

  void ArrayAppend(const SquirrelObject &o);

  template<typename T>
  BOOL ArrayAppend(T item);

  // === END Arrays ===

  BOOL SetInstanceUP(SQUserPointer up);
  BOOL IsNull() const;
  int IsNumeric() const;
  int Len() const;
  BOOL SetDelegate(SquirrelObject &obj);
  SquirrelObject GetDelegate();
  const SQChar* ToString();
  bool ToBool();
  SQInteger ToInteger();
  SQFloat ToFloat();
  SQUserPointer GetInstanceUP(SQUserPointer tag) const;
  SquirrelObject GetValue(const SQChar *key) const;
  BOOL Exists(const SQChar *key) const;
  FLOAT GetFloat(const SQChar *key) const;
  INT GetInt(const SQChar *key) const;
  const SQChar *GetString(const SQChar *key) const;
  bool GetBool(const SQChar *key) const;
  SquirrelObject GetValue(INT key) const;
  FLOAT GetFloat(INT key) const;
  INT GetInt(INT key) const;
  const SQChar *GetString(INT key) const;
  bool GetBool(INT key) const;
  SquirrelObject GetAttributes(const SQChar *key = NULL);
  SQObjectType GetType();
  HSQOBJECT & GetObjectHandle() const {return *(HSQOBJECT*)&_o;}
  BOOL BeginIteration();
  BOOL Next(SquirrelObject &key,SquirrelObject &value);
  void EndIteration();

  BOOL GetTypeTag(SQUserPointer * typeTag);

  // === Get the type name of item/object through string key in a table or class. Returns NULL if the type name is not set (not an SqPlus registered type).
  const SQChar * GetTypeName(const SQChar * key);
  // === Get the type name of item/object through INT key in a table or class. Returns NULL if the type name is not set (not an SqPlus registered type).
  const SQChar * GetTypeName(INT key);
  // === Get the type name of this object, else return NULL if not an SqPlus registered type.
  const SQChar * GetTypeName(void);

  // === Return base class of object using sq_getbase() === 
  SquirrelObject GetBase();
  
  // === BEGIN code suggestion from the Wiki ===
  // Get any bound type from this SquirrelObject. Note that Squirrel's handling of references and pointers still holds here.
  template<typename _ty>
  _ty Get(void);

  // Set any bound type to this SquirrelObject. Note that Squirrel's handling of references and pointers still holds here.
  template<typename _ty>
  SquirrelObject SetByValue(_ty val); // classes/structs should be passed by ref (below) to avoid an extra copy.

  // Set any bound type to this SquirrelObject. Note that Squirrel's handling of references and pointers still holds here.
  template<typename _ty>
  SquirrelObject &Set(_ty & val);

  // === END code suggestion from the Wiki ===

private:
  BOOL GetSlot(const SQChar *name) const;
  BOOL RawGetSlot(const SQChar *name) const;
  BOOL GetSlot(INT key) const;
  HSQOBJECT _o;
};

// VC1: SquirrelObject.cpp in the original, out of source order. LTCG
// passes `this` in EAX/EBX/ESI and the stdcall-style key on the stack;
// see the asmcmp report for each.
// TEMPLATE: VC1 0x004013e0
// ??$?0V?$KdPtr@UIUnknown@vc1@@@vc1@@@SquirrelObject@@QAE@ABV?$KdPtr@UIUnknown@vc1@@@vc1@@@Z
// TEMPLATE: VC1 0x00401400
// ??$?0UVector2@vc1@@@SquirrelObject@@QAE@ABUVector2@vc1@@@Z
// FUNCTION: VC1 0x00401de0
// ?GetInstanceUp@StackHandler@@QAEPAXHPAX@Z
// SYNTHETIC: VC1 0x00401e00
// SquirrelObject::`scalar deleting destructor'

// FUNCTION: VC1 0x004290e0
// ?Return@StackHandler@@QAEHPBD@Z
// FUNCTION: VC1 0x00429100
// ?GetString@StackHandler@@QAEPBDH@Z
// FUNCTION: VC1 0x00429150
// ?Return@StackHandler@@QAEHH@Z
// FUNCTION: VC1 0x0042f010
// ?Return@StackHandler@@QAEHM@Z
// FUNCTION: VC1 0x0042f030
// ?Return@StackHandler@@QAEH_N@Z
// FUNCTION: VC1 0x0042f080
// ?Return@StackHandler@@QAEHPAX@Z
// FUNCTION: VC1 0x0049a840
// ?GetInstanceUP@SquirrelObject@@QBEPAXPAX@Z

// FUNCTION: VC1 0x0049a910
// ?GetSlot@SquirrelObject@@ABEHPBD@Z

// FUNCTION: VC1 0x0049a9e0
// ?GetSlot@SquirrelObject@@ABEHH@Z

// FUNCTION: VC1 0x0049aaa0
// ?AttachToStackObject@SquirrelObject@@QAEXH@Z

// FUNCTION: VC1 0x0049ab30
// ??4SquirrelObject@@QAEAAV0@ABV0@@Z

// FUNCTION: VC1 0x0049abe0
// ??0SquirrelObject@@QAE@UtagSQObject@@@Z
// FUNCTION: VC1 0x0049ac30
// ??0SquirrelObject@@QAE@ABV0@@Z

// FUNCTION: VC1 0x0049ac80
// ??1SquirrelObject@@UAE@XZ

// FUNCTION: VC1 0x0049acc0
// ?GetInt@SquirrelObject@@QBEHPBD@Z

// FUNCTION: VC1 0x0049ad30
// ?GetValue@SquirrelObject@@QBE?AV1@PBD@Z

// FUNCTION: VC1 0x0049ad90
// ?GetString@SquirrelObject@@QBEPBDH@Z

// FUNCTION: VC1 0x0049ae30
// ?RawGetUserData@SquirrelObject@@QAEHPBDPAPAX1@Z

// FUNCTION: VC1 0x0049af00
// ?GetUserData@SquirrelObject@@QAEHPBDPAPAX1@Z
// FUNCTION: VC1 0x0049afb0
// ?NewUserData@SquirrelObject@@QAEHPBDHPAPAX@Z
// FUNCTION: VC1 0x0049b020
// ?SetValue@SquirrelObject@@QAEHPBD_N@Z

// FUNCTION: VC1 0x0049b0c0
// ?SetValue@SquirrelObject@@QAEHPBDM@Z

// FUNCTION: VC1 0x0049b130
// ?SetValue@SquirrelObject@@QAEHPBDH@Z

// FUNCTION: VC1 0x0049b1d0
// ?SetValue@SquirrelObject@@QAEHPBDABV1@@Z

// FUNCTION: VC1 0x0049b250
// ?SetValue@SquirrelObject@@QAEHHPBD@Z

// FUNCTION: VC1 0x0049b2f0
// ?GetTypeName@SquirrelObject@@QAEPBDXZ


struct StackHandler {
  StackHandler(HSQUIRRELVM v) {
    _top = sq_gettop(v);
    this->v = v;
  }
  SQFloat GetFloat(int idx) {
    SQFloat x = 0.0f;
    if(idx > 0 && idx <= _top) {
      sq_getfloat(v,idx,&x);
    }
    return x;
  }
  SQInteger GetInt(int idx) {
    SQInteger x = 0;
    if(idx > 0 && idx <= _top) {
      sq_getinteger(v,idx,&x);
    }
    return x;
  }
  HSQOBJECT GetObjectHandle(int idx) {
    HSQOBJECT x;
    if(idx > 0 && idx <= _top) {
      sq_resetobject(&x);
      sq_getstackobj(v,idx,&x);
    }
    return x;
  }
  const SQChar *GetString(int idx)
  {
    const SQChar *x = NULL;
    if(idx > 0 && idx <= _top) {
      sq_getstring(v,idx,&x);
    }
    return x;
  }
  SQUserPointer GetUserPointer(int idx)
  {
    SQUserPointer x = 0;
    if(idx > 0 && idx <= _top) {
      sq_getuserpointer(v,idx,&x);
    }
    return x;
  }
  SQUserPointer GetInstanceUp(int idx,SQUserPointer tag)
  {
    SQUserPointer self;
    if(SQ_FAILED(sq_getinstanceup(v,idx,(SQUserPointer*)&self,tag)))
      return NULL;
    return self;
  }
  SQUserPointer GetUserData(int idx,SQUserPointer tag=0)
  {
    SQUserPointer otag;
    SQUserPointer up;
    if(idx > 0 && idx <= _top) {
      if(SQ_SUCCEEDED(sq_getuserdata(v,idx,&up,&otag))) {
        if(tag == otag)
          return up;
      }
    }
    return NULL;
  }
  BOOL GetBool(int idx)
  {
    SQBool ret;
    if(idx > 0 && idx <= _top) {
      if(SQ_SUCCEEDED(sq_getbool(v,idx,&ret)))
        return ret;
    }
    return FALSE;
  }
  int GetType(int idx)
  {
    if(idx > 0 && idx <= _top) {
      return sq_gettype(v,idx);
    }
    return -1;
  }
  
  int GetParamCount() {
    return _top;
  }
  int Return(const SQChar *s)
  {
    sq_pushstring(v,s,-1);
    return 1;
  }
  int Return(FLOAT f)
  {
    sq_pushfloat(v,f);
    return 1;
  }
  int Return(INT i)
  {
    sq_pushinteger(v,i);
    return 1;
	}
	int Return(bool b)
	{
		sq_pushbool(v,b);
		return 1;
	}
  int Return(SQUserPointer p) {
    sq_pushuserpointer(v,p);
    return 1;
  }
	int Return(SquirrelObject &o)
	{
		sq_pushobject(v,o.GetObjectHandle());
		return 1;
	}
	int Return() { return 0; }
	int ThrowError(const SQChar *error) {
		return sq_throwerror(v,error);
	}
  HSQUIRRELVM GetVMPtr() { return v; }
private:
	int _top;
	HSQUIRRELVM v;
};

#endif //_SQUIRREL_OBJECT_H_
