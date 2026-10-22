// Included inside namespace vc1 by the transitional Windows file backend.
// Modern FILE is opaque: own mapping handles here instead of borrowing VC9's
// FILE::_tmpfname slot. This code is not an original-binary mapping.
struct SdlFileMapping
{
    FILE* file;
    void* view;
    HANDLE mapping;
    SdlFileMapping* next;
};
static SdlFileMapping* sdlFileMappings = 0;

static void KdFileUnmap(KdObject*, int handle, void* view)
{
    if (!view) return;
    SdlFileMapping** link = &sdlFileMappings;
    while (*link) {
        SdlFileMapping* item = *link;
        if (item->file == reinterpret_cast<FILE*>(handle) && item->view == view) {
            UnmapViewOfFile(item->view);
            CloseHandle(item->mapping);
            *link = item->next;
            free(item);
            return;
        }
        link = &item->next;
    }
}

static void* KdFileMap(KdObject*, int handle, unsigned* size)
{
    FILE* file = reinterpret_cast<FILE*>(handle);
    HANDLE os = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file)));
    HANDLE mapping = CreateFileMappingA(os, 0, PAGE_READONLY, 0, 0, 0);
    if (!mapping) return 0;
    void* view = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    SdlFileMapping* item = view
        ? static_cast<SdlFileMapping*>(malloc(sizeof(SdlFileMapping))) : 0;
    if (!item) {
        if (view) UnmapViewOfFile(view);
        CloseHandle(mapping);
        return 0;
    }
    item->file = file;
    item->view = view;
    item->mapping = mapping;
    item->next = sdlFileMappings;
    sdlFileMappings = item;
    *size = GetFileSize(os, 0);
    return view;
}
