#include "kd/blob/blob.h"

#include "kd/iface/iid.h"
#include "bindings/sqplus.h"

#include <new>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// STRING: VC1 0x004d4b78
static const char kBlob[] = "Blob";
// GLOBAL: VC1 0x004d0680
const char* g_kdNameBlob = kBlob;

// STRING: VC1 0x004d56c0
static const char kScriptSetHeaderSize[] = "SetHeaderSize";

// MOV EAX, "Blob"; RET. IScriptObject+0x14.
// FUNCTION: VC1 0x00417d40
const char* KdBlob::GetTypeName()
{
    return kBlob;
}

// RET 4; script "SetHeaderSize".
// FUNCTION: VC1 0x0046c970
void KdBlob::SetHeaderSize(int size)
{
    headerSize = size;
}

// XOR AL, AL; RET 8. IScriptObject+0x10: no writable members.
// FUNCTION: VC1 0x0046c980
unsigned char KdBlob::SetMember(void*, void*)
{
    return 0;
}

// ADD ECX, 0x1c; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x0046ca20
unsigned KdBlob::Release()
{
    unsigned value = --refs;
    if (value == 0) {
        delete static_cast<IAbstract*>(this);
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x0046ca40
unsigned KdBlob::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// IWriteable+0x0c. The size header, then the bytes.
// FUNCTION: VC1 0x0046ca50
bool KdBlob::WriteTo(const KdPtr<IOutputStream>& stream)
{
    unsigned size = data.items.size;

    if (stream.ptr->Write(&size, headerSize) != static_cast<unsigned>(headerSize)) {
        return false;
    }
    return stream.ptr->Write(data.items.begin, size) == size;
}

// RET 8. IInputStream+0x0c: up to size bytes from the position on (a null
// buffer skips them).
// FUNCTION: VC1 0x0046cab0
unsigned KdBlob::Read(void* buffer, unsigned size)
{
    unsigned count = data.items.size - position;

    if (size < count) {
        count = size;
    }
    if (buffer) {
        unsigned char* to = static_cast<unsigned char*>(buffer);
        const unsigned char* from = data.Data() + position;
        unsigned n;
        for (n = count; n != 0; --n) {
            *to++ = *from++;
        }
    }
    position += count;
    return count;
}

// RET 4. IScriptLoadable+0x0c: the bytes from a hex string.
// FUNCTION: VC1 0x0046cb00
bool KdBlob::LoadFromScript(SquirrelObject& table)
{
    const char* text;
    char digits[3];
    unsigned i;

    if (table.GetObjectHandle()._type != OT_STRING) {
        return false;
    }
    text = table.ToString();
    digits[0] = 0;
    digits[1] = 0;
    digits[2] = 0;
    data.Resize(strlen(text) / 2);
    for (i = 0; i < data.items.size; ++i) {
        digits[0] = text[i * 2];
        digits[1] = text[i * 2 + 1];
        data[i] = static_cast<unsigned char>(strtol(digits, 0, 16));
    }
    return true;
}

// IReadable+0x0c. The size header, then the bytes; rewinds the position.
// FUNCTION: VC1 0x0046cbb0
bool KdBlob::ReadFrom(const KdPtr<IInputStream>& stream)
{
    unsigned size = 0;

    if (stream.ptr->Read(&size, headerSize) != static_cast<unsigned>(headerSize)) {
        return false;
    }
    position = 0;
    data.Resize(size);
    return stream.ptr->Read(data.items.begin, size) == size;
}

// RET 8. IOutputStream+0x0c: writes at the position, growing the buffer;
// the size always becomes the end of the write.
// FUNCTION: VC1 0x0046cc30
unsigned KdBlob::Write(const void* buffer, unsigned size)
{
    const unsigned end = position + size;
    unsigned char* to;
    const unsigned char* from;
    unsigned n;

    if (data.items.capacity < end) {
        GrowCountedBuffer(&data.items, static_cast<int>(end), 1);
    }
    data.items.size = end;
    to = data.Data() + position;
    from = static_cast<const unsigned char*>(buffer);
    for (n = size; n != 0; --n) {
        *to++ = *from++;
    }
    position += size;
    return size;
}

// ESI=this.
// FUNCTION: VC1 0x0046cc80
KdBlob::~KdBlob() throw()
{
}

// EAX=this.
// FUNCTION: VC1 0x0046ccf0
KdBlob::KdBlob() throw()
    : position(0), headerSize(4)
{
}

// RET 4. Returns the new blob as IAbstract.
// FUNCTION: VC1 0x0046cd80
void** CreateKdBlob(void** out)
{
    void* const memory = malloc(sizeof(KdBlob));
    KdBlob* const object = memory ? new (memory) KdBlob() : 0;
    IUnknown* queried = 0;

    if (object) {
        queried = static_cast<IUnknown*>(
            static_cast<IAbstract*>(object)->QueryInterface(&g_iidAbstract));
    }
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

// RET 4 (hidden return). IScriptSaveable+0x0c: the bytes as a lowercase hex
// string, two digits per byte.
// FUNCTION: VC1 0x0046cdd0
SquirrelObject KdBlob::SaveToScript()
{
    char* const text = static_cast<char*>(malloc(data.items.size * 2 + 1));
    unsigned i;

    for (i = 0; i < data.items.size; ++i) {
        char* const digits = text + i * 2;
        _itoa(data[i], digits, 16);
        if (strlen(digits) != 2) {
            text[i * 2 + 1] = digits[0];
            digits[0] = '0';
        }
    }
    text[i * 2] = 0;
    SquirrelObject result;
    result.SetByValue(text);
    free(text);
    return result;
}

// RET 8. The method table (table 0x004ee510); there is no script instance.
// FUNCTION: VC1 0x0046cf50
unsigned char KdBlob::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptSetHeaderSize] = CreateScriptMethod<KdBlob>(&KdBlob::SetHeaderSize);
    }
    return methods.Find(name, out);
}

}
