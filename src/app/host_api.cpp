#include "kdcore/native.h"
#include "kdcore/runtime.h"

#include "bindings/component.h"
#include "bindings/matrix3.h"
#include "bindings/matrix4.h"
#include "bindings/rect.h"
#include "bindings/rectf.h"
#include "bindings/sqplus.h"
#include "bindings/vector4.h"
#include "kd/factory.h"
#include "kd/iface/icomponent_faces.h"

#include <math.h>
#include <stdlib.h>

#include <sqstdaux.h>

namespace vc1 {

// The g5 script natives that InitHostServices (0x004b61b0) registers on the
// root table with SqPlus::RegisterGlobal. The C++ names are the script names;
// the helpers' names are inferred. Component arguments arrive as the
// userpointer's IAbstract face (SqPlus Get, component.h).

// GLOBAL: VC1 0x004d75d8
extern const char kCantCastTo[] = "can't cast to %s";
// GLOBAL: VC1 0x004d75fc
extern const char kFailed[] = "%s failed";
// GLOBAL: VC1 0x004d7a14
extern const char kReadSimple[] = "g5::readSimple";
// GLOBAL: VC1 0x004d7a24
extern const char kWriteSimple[] = "g5::writeSimple";

// The empty name getIdentificator returns for a non-IIdentifiable.
// GLOBAL: VC1 0x004e9994
KdString g_kdEmptyName = KdEmptyText();

// The null component the KdPtr<IUnknown> natives return on a failed cast.
// GLOBAL: VC1 0x004ed70c
KdPtr<IUnknown> g_kdNullComponent;

// Never written; getBoundingRect passes its address to GetBoundingRect.
// GLOBAL: VC1 0x004ed6b0
RenderMask g_defaultRenderMask;

// Inferred name. The cast inlined into every component native: the T face of
// the script argument, AddRef'd; on failure "can't cast to <iid>" and the
// script call stack.
template <class T>
class ScriptFace : public KdPtr<T>
{
public:
    explicit ScriptFace(const KdPtr<IAbstract>& object) : KdPtr<T>(object)
    {
        if (!this->ptr) {
            HostLogError(&g_kdLogPrefix, kCantCastTo, *KdIid<T>::Get());
            sqstd_printcallstack(SquirrelVM::_VM);
        }
    }
};

// The system singleton through the factory, as ISystem. Inferred name.
// FUNCTION: VC1 0x00405410
KdPtr<ISystem> GetKdSystem()
{
    void* found;
    IUnknown* object;

    GetKdFactory()->QueryNamed(&found, &g_kdNameSystem);
    object = static_cast<IUnknown*>(found);
    if (!object) {
        return KdPtr<ISystem>();
    }
    KdPtr<ISystem> system(static_cast<ISystem*>(object->QueryInterface(&g_iidSystem)));
    object->Release();
    return system;
}

// FUNCTION: VC1 0x00408770
int randNum(int range)
{
    if (range <= 0) {
        return 0;
    }
    return static_cast<int>(static_cast<__int64>(rand()) * range / 0x8000);
}

// FUNCTION: VC1 0x004087a0
float randVal(float low, float high)
{
    float t = static_cast<float>(rand() / 32767.0);
    return low + (high - low) * t;
}

// FUNCTION: VC1 0x0040a400
int getTime()
{
    return static_cast<int>(GetKdSystem()->GetTime());
}

// 2*pi and pi as float literals widened to double.
static const double kTwoPi = 6.2831854820251465;
static const double kPi = 3.1415927410125732;

float WrapAngle(float angle);

// FUNCTION: VC1 0x0040cad0
float angleDelta(float from, float to)
{
    float delta = WrapAngle(from - to);
    if (delta > kPi) {
        delta = static_cast<float>(delta - kTwoPi);
    }
    return delta;
}

// FUNCTION: VC1 0x0040cb60
__declspec(noinline) float fltsign(float value)
{
    if (value > 0.0f) {
        return 1.0f;
    }
    if (value < 0.0f) {
        return -1.0f;
    }
    return 0.0f;
}

// Angle into [0, 2*pi). Inferred name.
// FUNCTION: VC1 0x0040ccb0
float WrapAngle(float angle)
{
    if (angle >= kTwoPi || angle < 0.0f) {
        float turns = static_cast<float>(floor(static_cast<float>(angle / kTwoPi)));
        angle = static_cast<float>(angle - turns * kTwoPi);
    }
    return angle;
}

// Milliseconds of QPC time.
// FUNCTION: VC1 0x00429190
int getSystemTime()
{
    LARGE_INTEGER qpc;

    QueryPerformanceCounter(&qpc);
    return static_cast<int>(static_cast<unsigned __int64>(
        static_cast<__int64>(static_cast<double>(qpc.QuadPart) * g_nsPerTick)) / 1000000);
}

// EAX=out. A 4-byte length then the bytes. Inferred name.
// FUNCTION: VC1 0x004291c0
static bool ReadKdString(KdString& out, const KdPtr<IInputStream>& stream)
{
    unsigned length = 0;

    if (stream->Read(&length, 4) != 4) {
        return false;
    }
    out.Resize(length);
    return stream->Read(out.begin, length) == length;
}

// FUNCTION: VC1 0x00429250
int readInt(const KdPtr<IAbstract>& object)
{
    int value;
    ScriptFace<IInputStream> stream(object);

    if (stream.ptr && stream->Read(&value, 4) != 4) {
        HostLogError(&g_kdLogPrefix, kFailed, kReadSimple);
    }
    return value;
}

// FUNCTION: VC1 0x004292e0
int readShort(const KdPtr<IAbstract>& object)
{
    short value;
    ScriptFace<IInputStream> stream(object);

    if (stream.ptr && stream->Read(&value, 2) != 2) {
        HostLogError(&g_kdLogPrefix, kFailed, kReadSimple);
    }
    return value;
}

// FUNCTION: VC1 0x00429370
int readUnsignedShort(const KdPtr<IAbstract>& object)
{
    unsigned short value;
    ScriptFace<IInputStream> stream(object);

    if (stream.ptr && stream->Read(&value, 2) != 2) {
        HostLogError(&g_kdLogPrefix, kFailed, kReadSimple);
    }
    return value;
}

// FUNCTION: VC1 0x00429400
int readByte(const KdPtr<IAbstract>& object)
{
    signed char value;
    ScriptFace<IInputStream> stream(object);

    if (stream.ptr && stream->Read(&value, 1) != 1) {
        HostLogError(&g_kdLogPrefix, kFailed, kReadSimple);
    }
    return value;
}

// FUNCTION: VC1 0x00429490
int readUnsignedByte(const KdPtr<IAbstract>& object)
{
    unsigned char value;
    ScriptFace<IInputStream> stream(object);

    if (stream.ptr && stream->Read(&value, 1) != 1) {
        HostLogError(&g_kdLogPrefix, kFailed, kReadSimple);
    }
    return value;
}

// FUNCTION: VC1 0x00429520
bool readBool(const KdPtr<IAbstract>& object)
{
    bool value;
    ScriptFace<IInputStream> stream(object);

    if (stream.ptr && stream->Read(&value, 1) != 1) {
        HostLogError(&g_kdLogPrefix, kFailed, kReadSimple);
    }
    return value;
}

// FUNCTION: VC1 0x004295b0
KdString readString(const KdPtr<IAbstract>& object)
{
    KdString value;
    ScriptFace<IInputStream> stream(object);

    if (stream.ptr && !ReadKdString(value, stream)) {
        HostLogError(&g_kdLogPrefix, kFailed, kReadSimple);
    }
    return value;
}

// FUNCTION: VC1 0x004296b0
void writeInt(const KdPtr<IAbstract>& object, int value)
{
    ScriptFace<IOutputStream> stream(object);

    if (stream.ptr && stream->Write(&value, 4) != 4) {
        HostLogError(&g_kdLogPrefix, kFailed, kWriteSimple);
    }
}

// FUNCTION: VC1 0x00429740
void writeShort(const KdPtr<IAbstract>& object, int value)
{
    short data = static_cast<short>(value);
    ScriptFace<IOutputStream> stream(object);

    if (stream.ptr && stream->Write(&data, 2) != 2) {
        HostLogError(&g_kdLogPrefix, kFailed, kWriteSimple);
    }
}

// FUNCTION: VC1 0x004297e0
void writeByte(const KdPtr<IAbstract>& object, int value)
{
    char data = static_cast<char>(value);
    ScriptFace<IOutputStream> stream(object);

    if (stream.ptr && stream->Write(&data, 1) != 1) {
        HostLogError(&g_kdLogPrefix, kFailed, kWriteSimple);
    }
}

// FUNCTION: VC1 0x00429870
void writeBool(const KdPtr<IAbstract>& object, bool value)
{
    ScriptFace<IOutputStream> stream(object);

    if (stream.ptr && stream->Write(&value, 1) != 1) {
        HostLogError(&g_kdLogPrefix, kFailed, kWriteSimple);
    }
}

// FUNCTION: VC1 0x00429900
void writeString(const KdPtr<IAbstract>& object, const KdString& text)
{
    ScriptFace<IOutputStream> stream(object);
    bool failed;

    if (!stream.ptr) {
        return;
    }
    {
        KdString copy(text);
        unsigned length = copy.size;
        failed = !(stream->Write(&length, 4) == 4
            && stream->Write(copy.begin, length) == length);
    }
    if (failed) {
        HostLogError(&g_kdLogPrefix, kFailed, kWriteSimple);
    }
}

// FUNCTION: VC1 0x004b4950
bool fileExists(const char* path)
{
    KdFileStat st;
    return StatKdPath(path, &st) == 0;
}

// QIs with a local copy of the name literal; no AddRef.
// FUNCTION: VC1 0x004b4970
bool isIdentifiable(const KdPtr<IAbstract>& object)
{
    const char* iid = kIidIdentifiable;
    return object.ptr && object.ptr->QueryInterface(&iid) != 0;
}

// FUNCTION: VC1 0x004b49a0
void cancelTimer(int timer)
{
    if (timer) {
        UnlinkTimer(reinterpret_cast<KdTimer*>(timer));
        free(reinterpret_cast<KdTimer*>(timer));
    }
}

// No cast report: a non-IClassifiable only passes an empty requirement.
// FUNCTION: VC1 0x004b49c0
bool checkMask(const KdPtr<IAbstract>& object, int required, int forbidden)
{
    KdPtr<IClassifiable> classes(object);
    unsigned mask;

    if (!classes.ptr) {
        return required == 0;
    }
    mask = classes->GetClassificators();
    return (mask & required) == static_cast<unsigned>(required)
        && (mask & forbidden) == 0;
}

// FUNCTION: VC1 0x004b4a30
void quit()
{
    KdEvent* event = AllocEvent();
    event->type = KD_EVENT_QUIT;
    QueueEvent(event);
}

// The log natives print the tostring of argument 2 and leave it pushed.
// FUNCTION: VC1 0x004b4a50
void logInfo(HSQUIRRELVM v)
{
    const SQChar* text;

    sq_tostring(v, 2);
    sq_getstring(v, -1, &text);
    HostLogInfo(&g_kdLogPrefix, text);
}

// FUNCTION: VC1 0x004b4af0
void logError(HSQUIRRELVM v)
{
    const SQChar* text;

    sq_tostring(v, 2);
    sq_getstring(v, -1, &text);
    HostLogError(&g_kdLogPrefix, text);
}

// FUNCTION: VC1 0x004b4b90
void logWarning(HSQUIRRELVM v)
{
    const SQChar* text;

    sq_tostring(v, 2);
    sq_getstring(v, -1, &text);
    HostLogWarning(&g_kdLogPrefix, text);
}

// FUNCTION: VC1 0x004b4c30
void logMessage(HSQUIRRELVM v)
{
    const SQChar* text;

    sq_tostring(v, 2);
    sq_getstring(v, -1, &text);
    HostLog(&g_kdLogPrefix, text);
}

// FUNCTION: VC1 0x004b4cf0
bool isExpired(const KdPtr<IAbstract>& object)
{
    ScriptFace<IWithLifeTime> life(object);
    return life.ptr ? life->IsExpired() : false;
}

// FUNCTION: VC1 0x004b4d70
unsigned getLifeTime(const KdPtr<IAbstract>& object)
{
    ScriptFace<IWithLifeTime> life(object);
    return life.ptr ? life->GetLifeTime() : 0;
}

// FUNCTION: VC1 0x004b4df0
RectF getBoundingRect(const KdPtr<IAbstract>& object)
{
    ScriptFace<IBoundingInfo> bounds(object);

    if (!bounds.ptr) {
        return RectF(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return bounds->GetBoundingRect(g_defaultRenderMask);
}

// FUNCTION: VC1 0x004b4ea0
KdPtr<IUnknown> getRenderTargetTexture(const KdPtr<IAbstract>& object)
{
    ScriptFace<IRenderTarget> target(object);
    IUnknown* texture;

    if (!target.ptr) {
        return g_kdNullComponent;
    }
    texture = *static_cast<IUnknown**>(target->GetRenderTexture());
    return KdPtr<IUnknown>(texture
        ? static_cast<IUnknown*>(texture->QueryInterface(&g_iidAbstract)) : 0);
}

// Ends the scene even when the cast failed.
// FUNCTION: VC1 0x004b4f70
void endRender(const KdPtr<IAbstract>& object)
{
    ScriptFace<IRenderTarget> target(object);
    target->EndFrame();
}

// FUNCTION: VC1 0x004b4fe0
KdPtr<IUnknown> beginRender(const KdPtr<IAbstract>& object, const KdPtr<IAbstract>& view)
{
    ScriptFace<IRenderTarget> target(object);
    IUnknown* graphics;

    if (!target.ptr) {
        return g_kdNullComponent;
    }
    KdPtr<ICamera> camera(view);
    KdPtr<IUnknown> drawn;
    graphics = *static_cast<IUnknown**>(target->BeginFrame(&drawn, &camera));
    return KdPtr<IUnknown>(graphics
        ? static_cast<IUnknown*>(graphics->QueryInterface(&g_iidAbstract)) : 0);
}

// FUNCTION: VC1 0x004b5110
void renderComponent(const KdPtr<IAbstract>& object, const KdPtr<IAbstract>& target)
{
    ScriptFace<IRenderable> renderable(object);

    if (renderable.ptr) {
        KdPtr<IGraphics> graphics(target);
        renderable->Render(&graphics);
    }
}

// FUNCTION: VC1 0x004b51c0
void updateComponent(const KdPtr<IAbstract>& object, int layer, int dt)
{
    ScriptFace<IUpdateable> updateable(object);

    if (updateable.ptr) {
        updateable->Update(static_cast<unsigned>(layer), dt);
    }
}

// FUNCTION: VC1 0x004b5230
void writeBuffer(const KdPtr<IAbstract>& object, const char* buffer, int size)
{
    ScriptFace<IOutputStream> stream(object);

    if (stream.ptr && stream->Write(buffer, size) != static_cast<unsigned>(size)) {
        // STRING: VC1 0x004d75ec
        HostLogError(&g_kdLogPrefix, kFailed, "g5::writeBuffer");
    }
}

// FUNCTION: VC1 0x004b52c0
bool writeComponent(const KdPtr<IAbstract>& output, const KdPtr<IAbstract>& object)
{
    ScriptFace<IOutputStream> stream(output);

    if (!stream.ptr) {
        return false;
    }
    ScriptFace<IWriteable> writeable(object);
    return writeable.ptr ? writeable->WriteTo(stream) : false;
}

// FUNCTION: VC1 0x004b53d0
bool readComponent(const KdPtr<IAbstract>& input, const KdPtr<IAbstract>& object)
{
    ScriptFace<IInputStream> stream(input);

    if (!stream.ptr) {
        return false;
    }
    ScriptFace<IReadable> readable(object);
    return readable.ptr ? readable->ReadFrom(stream) : false;
}

// FUNCTION: VC1 0x004b54e0
void setVisible(const KdPtr<IAbstract>& object, bool visible)
{
    ScriptFace<IVisible> face(object);

    if (face.ptr) {
        face->SetVisible(visible);
    }
}

// FUNCTION: VC1 0x004b5560
bool isVisible(const KdPtr<IAbstract>& object)
{
    ScriptFace<IVisible> face(object);
    return face.ptr ? face->IsVisible() : false;
}

// FUNCTION: VC1 0x004b55e0
bool setAnimation(const KdPtr<IAbstract>& object, const KdString& name, float speed)
{
    ScriptFace<IAnimateable> face(object);
    return face.ptr ? face->SetAnimation(name, speed) : false;
}

// FUNCTION: VC1 0x004b5670
void setPositionable(const KdPtr<IAbstract>& object, const KdPtr<IPositionable>& source)
{
    ScriptFace<IPositionableUser> face(object);

    if (face.ptr) {
        face->SetPositionable(source);
    }
}

// FUNCTION: VC1 0x004b56f0
void setPosition(const KdPtr<IAbstract>& object, const Matrix3& position)
{
    ScriptFace<IPositionable> face(object);

    if (face.ptr) {
        face->SetPosition(position);
    }
}

// FUNCTION: VC1 0x004b5770
const Matrix3& getPosition(const KdPtr<IAbstract>& object)
{
    ScriptFace<IPositionable> face(object);
    return face.ptr ? face->GetPosition() : Matrix3::Null;
}

// FUNCTION: VC1 0x004b57f0
int getClassificators(const KdPtr<IAbstract>& object)
{
    ScriptFace<IClassifiable> face(object);
    return face.ptr ? static_cast<int>(face->GetClassificators()) : 0;
}

// FUNCTION: VC1 0x004b5870
void removeClassificator(const KdPtr<IAbstract>& object, int mask)
{
    ScriptFace<IClassifiable> face(object);

    if (face.ptr) {
        face->RemoveClassificator(static_cast<unsigned>(mask));
    }
}

// FUNCTION: VC1 0x004b58f0
void addClassificator(const KdPtr<IAbstract>& object, int mask)
{
    ScriptFace<IClassifiable> face(object);

    if (face.ptr) {
        face->AddClassificator(static_cast<unsigned>(mask));
    }
}

// FUNCTION: VC1 0x004b5970
void setIdentificator(const KdPtr<IAbstract>& object, const KdString& name)
{
    ScriptFace<IIdentifiable> face(object);

    if (face.ptr) {
        face->SetName(name);
    }
}

// FUNCTION: VC1 0x004b59f0
const KdString& getIdentificator(const KdPtr<IAbstract>& object)
{
    ScriptFace<IIdentifiable> face(object);
    return face.ptr ? face->GetName() : g_kdEmptyName;
}

// FUNCTION: VC1 0x004b5a70
SquirrelObject saveToScript(const KdPtr<IAbstract>& object)
{
    ScriptFace<IScriptSaveable> face(object);

    if (!face.ptr) {
        return SquirrelObject();
    }
    return face->SaveToScript();
}

// FUNCTION: VC1 0x004b5b10
bool loadFromScript(const KdPtr<IAbstract>& object, SquirrelObject table)
{
    ScriptFace<IScriptLoadable> face(object);
    return face.ptr ? face->LoadFromScript(table) : false;
}

// FUNCTION: VC1 0x004b5b90
void setSlaveObject(const KdPtr<IAbstract>& object, const KdPtr<IAbstract>& slave)
{
    ScriptFace<IObjectControl> face(object);

    if (face.ptr) {
        face->SetSlaveObject(slave);
    }
}

// FUNCTION: VC1 0x004b5c10
bool isControlEnabled(const KdPtr<IAbstract>& object)
{
    ScriptFace<IManualControl> face(object);
    return face.ptr ? face->IsControlEnabled() : false;
}

// FUNCTION: VC1 0x004b5c90
void enableControl(const KdPtr<IAbstract>& object, bool enable)
{
    ScriptFace<IManualControl> face(object);

    if (face.ptr) {
        face->EnableControl(enable);
    }
}

// FUNCTION: VC1 0x004b5d10
KdPtr<IUnknown> cloneComponent(const KdPtr<IAbstract>& object)
{
    ScriptFace<ICloneable> face(object);

    if (!face.ptr) {
        return g_kdNullComponent;
    }
    KdPtr<IUnknown> copy;
    face->Clone(reinterpret_cast<void**>(&copy.ptr));
    return copy;
}

// FUNCTION: VC1 0x004b5db0
bool saveToFile(const KdPtr<IAbstract>& object, const char* path)
{
    ScriptFace<ISaveable> face(object);
    return face.ptr ? face->Save(path) : false;
}

// FUNCTION: VC1 0x004b5e30
bool loadFromFile(const KdPtr<IAbstract>& object, const char* path)
{
    ScriptFace<ILoadable> face(object);
    return face.ptr ? face->Load(path) != 0 : false;
}

// FUNCTION: VC1 0x004b5eb0
void shutdown(const KdPtr<IAbstract>& object)
{
    ScriptFace<IShutdownable> face(object);

    if (face.ptr) {
        face->Shutdown();
    }
}

// KD_EVENT_TIMER callback of setTimer: the userptr is the script closure.
// Inferred name.
// FUNCTION: VC1 0x004b5f20
static void ScriptTimerCallback(const KdEvent* event)
{
    HSQOBJECT closure;

    closure._type = OT_CLOSURE;
    closure._unVal.pClosure = static_cast<SQClosure*>(event->userptr);
    SquirrelVM::RunScript(SquirrelObject(closure));
}

// FUNCTION: VC1 0x004b5fe0
int setTimer(int milliseconds, SquirrelObject callback)
{
    KdTimer* timer;

    if (callback.GetType() != OT_CLOSURE) {
        // STRING: VC1 0x004d7608
        HostLogError(&g_kdLogPrefix, "invalid callback type");
        return 0;
    }
    timer = KdSetTimer(static_cast<__int64>(milliseconds) * 1000000,
        callback.GetObjectHandle()._unVal.pClosure);
    if (timer) {
        KdInstallCallback(ScriptTimerCallback, KD_EVENT_TIMER,
            callback.GetObjectHandle()._unVal.pClosure);
    }
    return reinterpret_cast<int>(timer);
}

// The registered component names, walked through the pointer-sorted table
// (each step re-finds the current name). g_kdLogPrefix doubles as the null
// past-the-end name.
// FUNCTION: VC1 0x004b6060
SquirrelObject getComponentIDs()
{
    SquirrelObject ids = SquirrelVM::CreateArray(0);
    KdFactory* factory = GetKdFactory();
    FactoryEntry* entry = factory->begin;
    FactoryEntry* end = reinterpret_cast<FactoryEntry*>(
        reinterpret_cast<char*>(factory->begin) + factory->size);
    const char* name = *(entry != end ? &entry->name : &g_kdLogPrefix);

    while (name) {
        FactoryEntry* first;
        FactoryEntry* last;

        ids.ArrayAppend(name);
        factory = GetKdFactory();
        first = factory->begin;
        last = reinterpret_cast<FactoryEntry*>(
            reinterpret_cast<char*>(factory->begin) + factory->size);
        end = last;
        while (first != last) {
            FactoryEntry* middle = first + (last - first) / 2;
            if (middle->name < name) {
                first = middle + 1;
            }
            else {
                last = middle;
            }
        }
        if (first < end && name < first->name) {
            first = end;
        }
        ++first;
        name = *(first != end ? &first->name : &g_kdLogPrefix);
    }
    return ids;
}

// FUNCTION: VC1 0x004b6190
KdPtr<IUnknown> getComponent(const char* name)
{
    return GetKdFactory()->Create(name);
}

// Runs every pixel (ARGB as 0..1 RGBA) through the matrix in place.
// FUNCTION: VC1 0x004ba930
void TextureFilter_ColorMatrix(const KdPtr<ITexture>& texture, const Matrix4& matrix)
{
    ITexture* target = texture.ptr;
    int height = target->GetHeight();
    int width = target->GetWidth();
    Rect region(0, 0, width, height);
    unsigned* pixels = static_cast<unsigned*>(target->Lock(&region, 0));
    unsigned count = static_cast<unsigned>(region.height * region.width);
    unsigned i;

    for (i = 0; i < count; ++i) {
        const unsigned color = pixels[i];
        const double scale = 1.0f / 255.0f;
        Vector4 mapped = matrix.Mult(Vector4(
            static_cast<float>(static_cast<int>((color >> 16) & 0xff) * scale),
            static_cast<float>(static_cast<int>((color >> 8) & 0xff) * scale),
            static_cast<float>(static_cast<int>(color & 0xff) * scale),
            static_cast<float>(static_cast<int>(color >> 24) * scale)));
        const double full = 255.0;
        pixels[i] = (static_cast<unsigned>(static_cast<unsigned char>(static_cast<int>(mapped.w * full))) << 24)
            | (static_cast<unsigned>(static_cast<unsigned char>(static_cast<int>(mapped.x * full))) << 16)
            | ((static_cast<unsigned>(static_cast<unsigned char>(static_cast<int>(mapped.y * full))) << 8)
                | static_cast<unsigned char>(static_cast<int>(mapped.z * full)));
    }
    target->Unlock();
}

}
