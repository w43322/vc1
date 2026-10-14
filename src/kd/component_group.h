#ifndef VC1_KD_COMPONENT_GROUP_H
#define VC1_KD_COMPONENT_GROUP_H

#include "kd/iface/icomponent_group.h"
#include "kd/factory.h"

namespace vc1 {

// Inferred name. One 0x14-byte entry of KdComponentGroup::records.
// SIZE 0x14
struct NamedRecord
{
    KdString name;          // 0x00
    KdRef<IUnknown> object; // 0x10

    NamedRecord() {}
    explicit NamedRecord(const KdString& key) : name(key) {}

private:
    NamedRecord(const NamedRecord&);
    NamedRecord& operator=(const NamedRecord&);
};

typedef char Vc1NamedRecordSizeCheck[sizeof(NamedRecord) == 0x14 ? 1 : -1];

// Inferred name. NamedRecord array in a KdFactory buffer (no vftable of its
// own). The out-of-line destructor destroys every record up to capacity.
// SIZE 0x10
struct KdRecordList
{
    KdFactory items; // 0x00

    ~KdRecordList() throw();
};

// Inferred name. Shared concrete IComponentGroup: a sorted name -> IUnknown*
// record table (0x14-byte records) plus a scratch IUnknown* list. Embedded in
// KdSystemBase at +0x0c and KdCompoundObject at +0x28. QueryInterface, AddRef
// and Release stay _purecall in its default vftable.
// SIZE 0x24
// VTABLE: VC1 0x004d4f2c
class KdComponentGroup : public IComponentGroup
{
public:
    KdRecordList records; // 0x04  sorted by name
    KdRefList extra;      // 0x14  IUnknown* list filled by Unmapped1c

    ~KdComponentGroup();

    int Shutdown();                         // vtable+0x0c
    int Unmapped10(void* key, void** out);  // vtable+0x10
    void** Unmapped14(void** out, void* key); // vtable+0x14
    void** Unmapped18(void** out, void* key); // vtable+0x18
    KdFactory* Unmapped1c(const char** iid);  // vtable+0x1c
    void* Unmapped20();                     // vtable+0x20
    int Unmapped24();                       // vtable+0x24
};

typedef char Vc1KdComponentGroupSizeCheck[sizeof(KdComponentGroup) == 0x24 ? 1 : -1];

extern void* g_groupNotify;

__declspec(noinline) unsigned char RecordEq(void* key, void* record);
IUnknown** InsertCountedSlot(void* key, void* factory) throw();
void AppendCountedRange(void* out, void* begin, void* end, void* factory);
unsigned char* LookupCountedRecord(void* factory, void* key);
unsigned char* FindCountedRecord(void* factory, void* key);

}

#endif
