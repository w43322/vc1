#ifndef VC1_BINDINGS_COMPONENT_H
#define VC1_BINDINGS_COMPONENT_H
#include "squirrel_object.h"
#include "memory/fixed_pool.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/icamera.h"
#include "kd/iface/ifont.h"
#include "kd/iface/igraphics.h"
#include "kd/iface/iid.h"
#include "kd/iface/irenderable.h"
#include "kd/iface/irender_target.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/itexture.h"

namespace vc1 {

struct IFont;
struct IFunctionalGroup;
struct IGridMap;
struct IGridRenderer;
struct IGridRouter;
struct IRoadRenderer;
struct ITileLayer;
struct ITileAnimationData;
struct IPositionable;
struct IRoute;
struct IInputStream;
struct IOutputStream;
struct ITexture;

// Inferred name. Script class "Component" (0x004cfb54), registered first by
// BindHostClasses through a temporary SQClassDef (0x0041dd80). Its SqPlus
// ClassType (0x004ee26c, vftable 0x004d74a8) has no type name, so this type
// has no TypeInfo. A script-side construct (0x0041fdd0) news a null handle in
// pools[1]; the release hook (0x004201b0) Releases it. Engine objects are
// pushed instead with an AddRef'd IAbstract* as the userpointer (Push
// 0x0040c970) and read back as IUnknown* (GetInstance 0x00401820).
struct ComponentRef
{
    IUnknown* ptr;

    ComponentRef() : ptr(0) {}
    // SqPlus ClassTypeCopyImpl<ComponentRef>::copy (0x00420170): AddRef the
    // new handle, Release the old one.
    ComponentRef& operator=(const ComponentRef& other)
    {
        if (other.ptr) {
            other.ptr->AddRef();
        }
        if (ptr) {
            ptr->Release();
        }
        ptr = other.ptr;
        return *this;
    }
    ~ComponentRef()
    {
        if (ptr) {
            ptr->Release();
        }
        ptr = 0;
    }

    VC1_SIZE_CLASS_NEW(ComponentRef)
};

typedef char Vc1ComponentRefSizeCheck[sizeof(ComponentRef) == 4 ? 1 : -1];

// Release hook of pushed engine objects (Push 0x0040c970).
SQInteger ReleaseComponent(SQUserPointer p, SQInteger size);

// Component metamethods, all registered with typemask "*". Names are the
// script names; the C++ names are inferred.
SQInteger ComponentCloned(HSQUIRRELVM vm);   // "_cloned"
SQInteger ComponentToString(HSQUIRRELVM vm); // "_tostring"
SQInteger ComponentCmp(HSQUIRRELVM vm);      // "_cmp"
SQInteger ComponentTypeOf(HSQUIRRELVM vm);   // "_typeof"
SQInteger ComponentNextI(HSQUIRRELVM vm);    // "_nexti"
SQInteger ComponentSet(HSQUIRRELVM vm);      // "_set"
SQInteger ComponentGet(HSQUIRRELVM vm);      // "_get"

// _nexti cursor into the iterated group's 0x14-byte name records.
extern unsigned char* g_componentCursor;
bool ComponentCursorIs(const char* key);

// Found by argument-dependent lookup from SqPlus's Call: a script array of
// the list's objects.
void Push(HSQUIRRELVM v, KdRefList& value);

template <>
struct KdIid<IGraphics>
{
    static const char** Get() { return &g_iidGraphics; }
};

}

// The instance type of the userpointer; only the name, Push is custom.
DECLARE_INSTANCE_TYPEINFO_NAME(vc1::IUnknown, Component)

namespace SqPlus {
template<> void Push(HSQUIRRELVM v, vc1::KdPtr<vc1::IUnknown>& value);
template<> void Push(HSQUIRRELVM v, vc1::KdPtr<vc1::IScriptObject>& value);
template<> void Push(HSQUIRRELVM v, vc1::KdPtr<vc1::IGraphics>& value);
// A returned render target goes out as a counted IUnknown (0x0041a890).
inline void Push(HSQUIRRELVM v, vc1::KdPtr<vc1::IRenderTarget>& value)
{
    vc1::KdPtr<vc1::IUnknown> object(value.ptr);
    Push(v, object);
}
// A returned texture (ObjectWatcher GetTexture) likewise.
inline void Push(HSQUIRRELVM v, vc1::KdPtr<vc1::ITexture>& value)
{
    vc1::KdPtr<vc1::IUnknown> object(value.ptr);
    Push(v, object);
}
// A returned IAbstract face (GetCamera's Call, 0x0041b390) likewise.
inline void Push(HSQUIRRELVM v, vc1::KdPtr<vc1::IAbstract>& value)
{
    vc1::KdPtr<vc1::IUnknown> object(value.ptr);
    Push(v, object);
}
// IGraphics GetFont / GetCamera return the slot; it goes out as a counted
// IUnknown (0x0040c970).
inline void Push(HSQUIRRELVM v, vc1::IUnknown** value)
{
    Push(v, *reinterpret_cast<vc1::KdPtr<vc1::IUnknown>*>(value));
}
// GetFont's slot is typed apart from GetCamera's: the original has two
// separate (byte-identical) dispatchers per class for them.
inline void Push(HSQUIRRELVM v, vc1::IFont** value)
{
    Push(v, *reinterpret_cast<vc1::KdPtr<vc1::IUnknown>*>(value));
}
// Inlined into SquirrelObject::Get (0x0041b830): the userpointer's IAbstract
// face, AddRef'd.
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >,
    HSQUIRRELVM v, int idx)
{
    vc1::IUnknown* self = GetInstance<vc1::IUnknown, false>(v, idx);
    // Null first: the original falls through to the null case.
    return vc1::KdPtr<vc1::IAbstract>(!self
        ? 0
        : static_cast<vc1::IAbstract*>(self->QueryInterface(&vc1::g_iidAbstract)));
}
// The g5 natives' component arguments (host_api.cpp): the IAbstract face,
// converted in the Call expression where the parameter is another face.
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::IAbstract>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::IPositionable>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::ITexture>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
// GameLevel SetDynamicLayer's group (Call 0x00403e50).
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::IFunctionalGroup>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::ICamera>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::IRenderable>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
// PassMap AddRenderer / RemoveRenderer, QI'd to IGridRenderer in the Call
// (0x00405e00).
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::IGridRenderer>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
// TerrainBuilder's setters, QI'd in their Calls (0x004048c0, 0x00407e00,
// 0x00407ea0, 0x00407fe0).
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::IGridMap>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::IGridRouter>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::IRoadRenderer>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::ITileLayer>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
// BuildingBuilder SetBuildingData's animation data (Call 0x004016b0).
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::ITileAnimationData>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
// TransportMover InitMoving's route and the stream arguments of
// Write/ReadTransportToStream (Calls 0x0040bcd0, 0x0040c1f0, 0x0040c370).
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::IRoute>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::IInputStream>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::IOutputStream>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
// SetFont's argument, QI'd to IFont in the Call (0x0041d040).
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::IFont>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
// A const KdPtr<IGraphics>& argument: the IAbstract face, converted to
// IGraphics in the Call expression, so both temporaries outlive the native
// (Calls 0x00403120, 0x00403240).
inline vc1::KdPtr<vc1::IAbstract> Get(TypeWrapper<const vc1::KdPtr<vc1::IGraphics>&>,
    HSQUIRRELVM v, int idx)
{
    return Get(TypeWrapper<vc1::KdPtr<vc1::IAbstract> >(), v, idx);
}
}

// SqPlus instantiations for Component.

// TEMPLATE: VC1 0x00401820
// ??$GetInstance@UIUnknown@vc1@@$0A@@SqPlus@@YAPAUIUnknown@vc1@@PAUSQVM@@H@Z

// TEMPLATE: VC1 0x004014a0
// ??$Set@V?$KdPtr@UIUnknown@vc1@@@vc1@@@SquirrelObject@@QAEAAV0@AAV?$KdPtr@UIUnknown@vc1@@@vc1@@@Z

// TEMPLATE: VC1 0x00403d30
// ??$Set@V?$KdPtr@UIScriptObject@vc1@@@vc1@@@SquirrelObject@@QAEAAV0@AAV?$KdPtr@UIScriptObject@vc1@@@vc1@@@Z

// TEMPLATE: VC1 0x00416550
// ??$Set@V?$KdPtr@UIGraphics@vc1@@@vc1@@@SquirrelObject@@QAEAAV0@AAV?$KdPtr@UIGraphics@vc1@@@vc1@@@Z

// TEMPLATE: VC1 0x0041b830
// ??$Get@V?$KdPtr@UIAbstract@vc1@@@vc1@@@SquirrelObject@@QAE?AV?$KdPtr@UIAbstract@vc1@@@vc1@@XZ

// TEMPLATE: VC1 0x0041dd80
// ??0?$SQClassDef@UComponentRef@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAE@PBD@Z

// TEMPLATE: VC1 0x0041ee90
// ??$RegisterClassType@UComponentRef@vc1@@@SqPlus@@YA?AVSquirrelObject@@PAUSQVM@@PBD1@Z

// TEMPLATE: VC1 0x0041fdd0
// ?construct@?$ConstructReleaseClass@UComponentRef@vc1@@@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004201b0
// ?release@?$ConstructReleaseClass@UComponentRef@vc1@@@SqPlus@@SAHPAXH@Z

// TEMPLATE: VC1 0x00423620
// ??$sq_typeof@UComponentRef@vc1@@@SqPlus@@YAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00423780
// ??$PostConstruct@UComponentRef@vc1@@@SqPlus@@YAHPAUSQVM@@PAUComponentRef@vc1@@P6AHPAXH@Z@Z

#endif
