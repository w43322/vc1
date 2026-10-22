#include "kd/resource/resource_manager.h"

#include "kd/factory.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/idisplay.h"
#include "kd/iface/iid.h"
#include "kd/iface/iloadable.h"
#include "kd/iface/ishutdownable.h"
#include "kd/iface/isound_driver.h"
#include "kd/iface/isystem.h"
#include "host/platform.h"
#include "bindings/component.h"
#include "bindings/sqplus.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

// STRING: VC1 0x004d4ba0
static const char kResourceManager[] = "ResourceManager";
// GLOBAL: VC1 0x004d0670
const char* g_kdNameResourceManager = kResourceManager;

// GLOBAL: VC1 0x004ed540
IAbstract* g_kdResourceManager;

// Inferred name. Never assigned; released at exit (0x004ca9b0). What a
// failed load returns and what FreeUnusedResources leaves in a freed record.
// GLOBAL: VC1 0x004ed6dc
KdRef<IUnknown> g_kdNullResource;

// Also read by 0x00461883 and 0x004ae6b4.
// GLOBAL: VC1 0x004d1230
extern const char kScriptGetTexture[] = "GetTexture";

// MOV EAX, "ResourceManager"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x00417910
const char* KdResourceManager::GetTypeName()
{
    return kResourceManager;
}

// Inlined into the scalar deleting destructor (0x00417920).
KdResourceManager::~KdResourceManager() throw()
{
    g_kdResourceManager = 0;
}

// EDI=out. Factory "ResourceManager" as IResourceManager (AddRef'd), or null.
// FUNCTION: VC1 0x00418010
IResourceManager** GetKdResourceManager(IResourceManager** out)
{
    void* unknown;
    IResourceManager* queried;

    GetKdFactory()->QueryNamed(&unknown, &g_kdNameResourceManager);
    if (!unknown) {
        *out = 0;
        return out;
    }
    queried = static_cast<IResourceManager*>(
        static_cast<IUnknown*>(unknown)->QueryInterface(&g_iidResourceManager));
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(unknown)->Release();
    return out;
}

// XOR AL,AL; RET 8. IScriptObject vtable+0x10.
// FUNCTION: VC1 0x0046d510
unsigned char KdResourceManager::SetMember(void* name, void* value)
{
    (void)name;
    (void)value;
    return 0;
}

// FUNCTION: VC1 0x0046d570
unsigned KdResourceManager::Release()
{
    return IAbstract::Release();
}

// FUNCTION: VC1 0x0046d590
unsigned KdResourceManager::AddRef()
{
    return IAbstract::AddRef();
}

// STRING: VC1 0x004d558c
static const char kUnloaded[] = "Unloaded %s";

// RET 4. Script "FreeResource": unregisters name and shuts the resource down.
// FUNCTION: VC1 0x0046d5a0
void KdResourceManager::FreeResource(const KdString& name)
{
    IComponentGroup* group = this;
    KdPtr<IShutdownable> resource(group->UnregisterObject(name));

    if (resource.ptr) {
        resource->Shutdown();
        HostLog(&g_kdNameResourceManager, kUnloaded, name.c_str());
    }
}

// STRING: VC1 0x004d55b8
static const char kIsntLoadable[] = "Isn't loadable: %s";
// STRING: VC1 0x004d55cc
static const char kCantLoad[] = "Can't load: %s";

// EDI=&object, EBX=&name, stack (manager, hidden return), RET 8. Loads
// object from name and registers it under name; g_kdNullResource on failure.
// FUNCTION: VC1 0x0046d630
static KdPtr<IUnknown> LoadResource(KdResourceManager* manager, KdPtr<IUnknown>& object,
    const KdString& name)
{
    ILoadable* loadable = object.ptr
        ? static_cast<ILoadable*>(object.ptr->QueryInterface(&g_iidLoadable))
        : 0;

    if (!loadable) {
        HostLogError(&g_kdNameResourceManager, kIsntLoadable, name.c_str());
        return KdPtr<IUnknown>(g_kdNullResource.ptr);
    }
    loadable->AddRef();
    if (!loadable->Load(name.c_str())) {
        HostLogError(&g_kdNameResourceManager, kCantLoad, name.c_str());
        KdPtr<IUnknown> failed(g_kdNullResource.ptr);
        loadable->Release();
        return failed;
    }
    IComponentGroup* group = manager;
    group->RegisterObject(name, reinterpret_cast<const KdPtr<IAbstract>&>(object));
    KdPtr<IUnknown> loaded(object.ptr);
    loadable->Release();
    return loaded;
}

// EAX=this. The group starts empty, refs at 1.
// FUNCTION: VC1 0x0046d740
KdResourceManager::KdResourceManager() throw()
{
}

// The singleton's IAbstract face, created on first use.
// FUNCTION: VC1 0x0046d7a0
void** CreateKdResourceManager(void** out)
{
    IUnknown* queried;
    KdResourceManager* object;

    if (g_kdResourceManager) {
        queried = static_cast<IUnknown*>(
            g_kdResourceManager->QueryInterface(&g_iidAbstract));
        *out = queried;
        if (queried) {
            queried->AddRef();
        }
        return out;
    }

    object = static_cast<KdResourceManager*>(malloc(sizeof(KdResourceManager)));
    if (!object) {
        g_kdResourceManager = 0;
        queried = 0;
    }
    else {
        new (object) KdResourceManager();
        g_kdResourceManager = static_cast<IAbstract*>(object);
        queried = static_cast<IUnknown*>(g_kdResourceManager->QueryInterface(&g_iidAbstract));
        if (queried) {
            queried->AddRef();
        }
    }
    queried->Release();
    *out = queried;
    queried->AddRef();
    queried->Release();
    return out;
}

// STRING: VC1 0x004d5598
static const char kUnusedDeleted[] = "FreeUnusedResources: %s deleted";

// Script "FreeUnusedResources": after the group's ClearCollected, every resource
// only the group still references is logged, dropped and erased.
// FUNCTION: VC1 0x0046d9a0
void KdResourceManager::FreeUnusedResources()
{
    IComponentGroup* group = this;
    unsigned char* it;

    group->ClearCollected();
    it = reinterpret_cast<unsigned char*>(records.items.begin);
    while (it != reinterpret_cast<unsigned char*>(records.items.begin) + records.items.size) {
        NamedRecord* record = reinterpret_cast<NamedRecord*>(it);

        if (record->object.ptr) {
            record->object.ptr->AddRef();
            if (record->object.ptr->Release() == 1) {
                unsigned char* begin;

                HostLog(&g_kdNameResourceManager, kUnusedDeleted, record->name.c_str());
                if (g_kdNullResource.ptr) {
                    g_kdNullResource.ptr->AddRef();
                }
                if (record->object.ptr) {
                    record->object.ptr->Release();
                }
                record->object.ptr = g_kdNullResource.ptr;
                begin = reinterpret_cast<unsigned char*>(records.items.begin);
                EraseCountedBytes(it, it + sizeof(NamedRecord), begin + records.items.size);
                records.items.size -= sizeof(NamedRecord);
                it += reinterpret_cast<unsigned char*>(records.items.begin) - begin;
                continue;
            }
        }
        it += sizeof(NamedRecord);
    }
}

// STRING: VC1 0x004d5544
static const char kScriptGetEffect[] = "GetEffect";
// STRING: VC1 0x004d5550
static const char kScriptGetSound[] = "GetSound";
// STRING: VC1 0x004d555c
static const char kScriptGetMusic[] = "GetMusic";
// STRING: VC1 0x004d5568
static const char kScriptFreeResource[] = "FreeResource";
// STRING: VC1 0x004d5578
static const char kScriptFreeUnusedResources[] = "FreeUnusedResources";

// RET 8. IScriptObject vtable+0x0c. Method table 0x004ee600 (guard bit
// 0x004ee610, atexit 0x004ca950).
// FUNCTION: VC1 0x0046db00
unsigned char KdResourceManager::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptGetTexture] = CreateScriptMethod<KdResourceManager>(&KdResourceManager::GetTexture);
        methods[kScriptGetFont] = CreateScriptMethod<KdResourceManager>(&KdResourceManager::GetFont);
        methods[kScriptGetEffect] = CreateScriptMethod<KdResourceManager>(&KdResourceManager::GetEffect);
        methods[kScriptGetSound] = CreateScriptMethod<KdResourceManager>(&KdResourceManager::GetSound);
        methods[kScriptGetMusic] = CreateScriptMethod<KdResourceManager>(&KdResourceManager::GetMusic);
        methods[kScriptFreeResource] = CreateScriptMethod<KdResourceManager>(&KdResourceManager::FreeResource);
        methods[kScriptFreeUnusedResources] = CreateScriptMethod<KdResourceManager>(&KdResourceManager::FreeUnusedResources);
    }
    return methods.Find(name, out);
}

// RET 8. A new music object from the "SoundDriver" (ISoundDriver
// vtable+0x14), loaded and registered on a miss.
// FUNCTION: VC1 0x0046de80
KdPtr<IUnknown> KdResourceManager::GetMusic(const KdString& name)
{
    IComponentGroup* group = this;
    KdPtr<IUnknown> found = group->FindObject(name);

    if (found.ptr) {
        return found;
    }
    KdPtr<IUnknown> unknown;
    GetKdFactory()->QueryNamed(reinterpret_cast<void**>(&unknown.ptr), &g_kdNameSoundDriver);
    ISoundDriver* driver = unknown.ptr
        ? static_cast<ISoundDriver*>(unknown.ptr->QueryInterface(&g_iidSoundDriver))
        : 0;
    if (driver) {
        driver->AddRef();
    }
    KdPtr<IUnknown> music;
    KdPtr<IUnknown> result = LoadResource(this,
        *static_cast<KdPtr<IUnknown>*>(driver->CreateMusic(&music)), name);
    driver->Release();
    return result;
}

// RET 8. As GetMusic with ISoundDriver vtable+0x10.
// FUNCTION: VC1 0x0046df70
KdPtr<IUnknown> KdResourceManager::GetSound(const KdString& name)
{
    IComponentGroup* group = this;
    KdPtr<IUnknown> found = group->FindObject(name);

    if (found.ptr) {
        return found;
    }
    KdPtr<IUnknown> unknown;
    GetKdFactory()->QueryNamed(reinterpret_cast<void**>(&unknown.ptr), &g_kdNameSoundDriver);
    ISoundDriver* driver = unknown.ptr
        ? static_cast<ISoundDriver*>(unknown.ptr->QueryInterface(&g_iidSoundDriver))
        : 0;
    if (driver) {
        driver->AddRef();
    }
    KdPtr<IUnknown> sound;
    KdPtr<IUnknown> result = LoadResource(this,
        *static_cast<KdPtr<IUnknown>*>(driver->CreateSound(&sound)), name);
    driver->Release();
    return result;
}

// RET 8. A new "Font" from the factory, loaded and registered on a miss.
// FUNCTION: VC1 0x0046e060
KdPtr<IUnknown> KdResourceManager::GetFont(const KdString& name)
{
    IComponentGroup* group = this;
    KdPtr<IUnknown> found = group->FindObject(name);

    if (found.ptr) {
        return found;
    }
    KdPtr<IUnknown> font;
    GetKdFactory()->QueryNamed(reinterpret_cast<void**>(&font.ptr), &g_kdNameFont);
    return LoadResource(this, font, name);
}

// RET 8. A new effect from the display (IDisplay vtable+0x3c).
// FUNCTION: VC1 0x0046e0f0
KdPtr<IUnknown> KdResourceManager::GetEffect(const KdString& name)
{
    IComponentGroup* group = this;
    KdPtr<IUnknown> found = group->FindObject(name);

    if (found.ptr) {
        return found;
    }
    KdPtr<ISystem> system(GetKdSystem());
    KdPtr<IUnknown> display;
    KdPtr<IUnknown> effect;
    IDisplay* face = *reinterpret_cast<IDisplay**>(
        system->GetDisplay(reinterpret_cast<void**>(&display.ptr)));
    return LoadResource(this,
        *reinterpret_cast<KdPtr<IUnknown>*>(face->CreateEffect(reinterpret_cast<void**>(&effect.ptr))),
        name);
}

// RET 8. A new texture from the display (IDisplay::CreateTexture).
// FUNCTION: VC1 0x0046e1d0
KdPtr<IUnknown> KdResourceManager::GetTexture(const KdString& name)
{
    IComponentGroup* group = this;
    KdPtr<IUnknown> found = group->FindObject(name);

    if (found.ptr) {
        return found;
    }
    KdPtr<ISystem> system(GetKdSystem());
    KdPtr<IUnknown> display;
    KdPtr<IUnknown> texture;
    IDisplay* face = *reinterpret_cast<IDisplay**>(
        system->GetDisplay(reinterpret_cast<void**>(&display.ptr)));
    return LoadResource(this,
        *reinterpret_cast<KdPtr<IUnknown>*>(face->CreateTexture(reinterpret_cast<void**>(&texture.ptr))),
        name);
}

}
