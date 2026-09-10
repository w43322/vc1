#ifndef VC1_KD_FACTORY_H
#define VC1_KD_FACTORY_H

#include "kd/iface/iunknown.h"

namespace vc1 {

class KdFactory;
struct FactoryEntry;
struct FactoryInsert;

typedef void** (*KdFactoryFn)(void** out);

// SIZE 0x08
struct FactoryEntry
{
    const char* name; // 0x00
    KdFactoryFn create; // 0x04
};

typedef char Vc1FactoryEntrySizeCheck[sizeof(FactoryEntry) == 0x08 ? 1 : -1];

// Sorted name/create table used by RunApp AcquireNamed.
// SIZE 0x10
// VTABLE: VC1 0x004cfb8c
class KdFactory
{
public:
    KdFactory() : begin(0), size(0), capacity(0) {}
    virtual ~KdFactory();     // vtable+0x00
    virtual void Clear();     // vtable+0x04
    virtual unsigned Count(); // vtable+0x08
    void** __stdcall QueryNamed(void** out, const char** name);

    FactoryEntry* begin; // 0x04
    unsigned size;       // 0x08
    unsigned capacity;   // 0x0c
};

typedef char Vc1KdFactorySizeCheck[sizeof(KdFactory) == 0x10 ? 1 : -1];

// SYNTHETIC: VC1 0x00401f30
// vc1::KdFactory::`scalar deleting destructor'

// SIZE 0x08
struct FactoryInsert
{
    FactoryEntry* entry;    // 0x00
    unsigned char inserted; // 0x04
};

typedef char Vc1FactoryInsertSizeCheck[sizeof(FactoryInsert) == 0x08 ? 1 : -1];

extern KdFactory g_kdFactory;
extern unsigned g_kdFactoryGuard;
extern IUnknown* g_kdDefaultUnknown;

extern const char* g_kdNameDisplay;
extern const char* g_kdNameTexture;
extern const char* g_kdNameSoundDriver;
extern const char* g_kdNameSystem;
extern const char* g_kdNameConsole;
extern const char* g_kdNameFont;
extern const char* g_kdNameLayer;
extern const char* g_kdNameUpdateGroup;
extern const char* g_kdNameRenderGroup;
extern const char* g_kdNameInputGroup;
extern const char* g_kdNameResourceManager;
extern const char* g_kdNameCompoundObject;
extern const char* g_kdNameFile;
extern const char* g_kdNameScene;
extern const char* g_kdNameBlob;
extern const char* g_kdNameCamera;
extern const char* g_kdNameScriptedObject;
extern const char* g_kdNameTileManager;
extern const char* g_kdNameGameLevel;
extern const char* g_kdNameGameObject;
extern const char* g_kdNameTileAnimationData;
extern const char* g_kdNameTileLayer;
extern const char* g_kdNameTileObject;
extern const char* g_kdNamePassMap;
extern const char* g_kdNameGridRouter;
extern const char* g_kdNameTerrainBuilder;
extern const char* g_kdNameRoadRenderer;
extern const char* g_kdNameTileAnimation;
extern const char* g_kdNameBuildingBuilder;
extern const char* g_kdNameRoute;
extern const char* g_kdNameTransportMover;
extern const char* g_kdNameGridRectRenderer;
extern const char* g_kdNameObjectWatcher;
extern const char* g_kdNameClassificatedRenderer;
extern const char* g_kdNameTspSolver;
extern const char* g_kdNameGroupRenderer;
extern const char* g_kdNameRoadCellsRenderer;

void** CreateKdDisplay(void** out);
void** CreateKdSoundDriver(void** out);
void** CreateKdSystem(void** out);
void** CreateKdConsole(void** out);
void** CreateKdFont(void** out);
void** CreateKdLayer(void** out);
void** CreateKdUpdateGroup(void** out);
void** CreateKdRenderGroup(void** out);
void** CreateKdInputGroup(void** out);
void** CreateKdResourceManager(void** out);
void** CreateKdCompoundObject(void** out);
void** CreateKdFile(void** out);
void** CreateKdScene(void** out);
void** CreateKdBlob(void** out);
void** CreateKdCamera(void** out);
void** CreateKdScriptedObject(void** out);
void** CreateKdTileManager(void** out);
void** CreateKdGameLevel(void** out);
void** CreateKdGameObject(void** out);
void** CreateKdTileAnimationData(void** out);
void** CreateKdTileLayer(void** out);
void** CreateKdTileObject(void** out);
void** CreateKdPassMap(void** out);
void** CreateKdGridRouter(void** out);
void** CreateKdTerrainBuilder(void** out);
void** CreateKdRoadRenderer(void** out);
void** CreateKdTileAnimation(void** out);
void** CreateKdBuildingBuilder(void** out);
void** CreateKdRoute(void** out);
void** CreateKdTransportMover(void** out);
void** CreateKdGridRectRenderer(void** out);
void** CreateKdObjectWatcher(void** out);
void** CreateKdClassificatedRenderer(void** out);
void** CreateKdTspSolver(void** out);
void** CreateKdGroupRenderer(void** out);
void** CreateKdRoadCellsRenderer(void** out);

FactoryInsert* RegisterFactory(FactoryEntry* entry, FactoryInsert* out);
FactoryEntry* FindFactoryEntry(KdFactory* factory, const char** key);
void ReserveFactoryTable(unsigned newCount);
void GrowCountedBuffer(void* self, int bytes, char exact);
void ReverseCountedBytes(void* first, void* last);
void EraseCountedBytes(void* first, void* middle, void* last);
void* InsertCountedBytes(void* self, void* pos, int bytes);
void* CopyCountedString(void* dest, void* source);
void AssignCountedString(void* self, const char* text, unsigned len);
void RegisterFactoryPair(const char** name, KdFactoryFn fn);
KdFactory* GetKdFactory();
void** QueryKdFactory(void** out, const char** name);
void DestroyKdFactory();
IUnknown* AcquireNamed(const char** factoryName, const char** iid);

}

#endif
