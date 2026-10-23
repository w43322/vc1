// The "file" driver's map slots (kdcore/startup.cpp). They park the mapping
// handle in the VC9 CRT's FILE::_tmpfname, so each target supplies its own.
#include "kdcore/runtime.h"

#include <io.h>
#include <stdio.h>

namespace vc1 {

// FUNCTION: VC1 0x0047cc90
void KdFileUnmap(KdObject*, int handle, void* view)
{
    if (view) {
        FILE* const file = reinterpret_cast<FILE*>(handle);
        UnmapViewOfFile(view);
        CloseHandle(reinterpret_cast<HANDLE>(file->_tmpfname));
        file->_tmpfname = 0;
    }
}

// The mapping handle is parked in FILE::_tmpfname (+0x1c) until unmap.
// FUNCTION: VC1 0x0047ccc0
void* KdFileMap(KdObject*, int handle, unsigned* size)
{
    FILE* const file = reinterpret_cast<FILE*>(handle);
    const HANDLE os = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file)));
    const HANDLE mapping = CreateFileMappingA(os, 0, PAGE_READONLY, 0, 0, 0);
    void* view;

    if (!mapping) {
        return 0;
    }
    view = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    if (!view) {
        CloseHandle(mapping);
        return 0;
    }
    *size = GetFileSize(os, 0);
    file->_tmpfname = reinterpret_cast<char*>(mapping);
    return view;
}

}
