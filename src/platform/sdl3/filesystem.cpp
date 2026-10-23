#include "host/platform.h"
#include "platform/sdl3/d3d8.h"
#include <SDL3/SDL.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#include <cerrno>
#include <ctime>
#include <climits>

namespace vc1 {
extern char g_resourceLocale[8];
extern const char kKdDataDir[]="data/";
struct File { SDL_IOStream* io;std::string path; };
static std::string preferencePath;
static File* Handle(int h) {return reinterpret_cast<File*>(h);}
char* CopyExeDir(const char* path,char* dest)
{
    if(!path || strncmp(path,"data/",5)) return const_cast<char*>(path);
    if(preferencePath.empty()) {
        char* dir=SDL_GetPrefPath("","Virtual City");
        if(!dir) return nullptr;
        preferencePath=dir;SDL_free(dir);
    }
    std::string full=preferencePath+(path+5);
    if(full.size()>=260) {errno=ENAMETOOLONG;return nullptr;}
    memcpy(dest,full.c_str(),full.size()+1);return dest;
}
static void Destroy(KdObject*) {}
// The texture file open for reading names the texture the display creates
// from it (texture mods, d3d8.cpp).
static File* imageFile;
static bool IsImage(const char* path,const char* mode)
{
    const char* ext=strrchr(path,'.');
    return !strchr(mode,'w') && !strchr(mode,'a') && ext &&
        (!SDL_strcasecmp(ext,".tga") || !SDL_strcasecmp(ext,".pvr") || !SDL_strcasecmp(ext,".jpg"));
}
static int Open(KdObject*,const char* path,const char* mode)
{
    const char* name=path;
    char translated[260];path=CopyExeDir(path,translated);if(!path) return 0;
    if(strchr(mode,'w') || strchr(mode,'a')) {
        std::string parent=path;size_t last=parent.find_last_of("/\\");
        if(last!=std::string::npos) SDL_CreateDirectory(parent.substr(0,last).c_str());
    }
    SDL_IOStream* io=SDL_IOFromFile(path,mode);
    if(!io) return 0;
    File* file=new File{io,path};
    if(IsImage(name,mode)) {imageFile=file;SDL_strlcpy(g_sdlImagePath,name,sizeof(g_sdlImagePath));}
    return reinterpret_cast<int>(file);
}
static int Close(KdObject*,int h)
{
    File* f=Handle(h);bool ok=SDL_CloseIO(f->io);
    if(f==imageFile) {imageFile=nullptr;g_sdlImagePath[0]=0;}
    delete f;return ok ? 0 : -1;
}
static int Flush(KdObject*,int h) {return SDL_FlushIO(Handle(h)->io) ? 0 : -1;}
static int Read(KdObject*,void* b,unsigned size,unsigned count,int h)
{
    if(!size || count>UINT_MAX/size) return 0;
    return static_cast<int>(SDL_ReadIO(Handle(h)->io,b,size_t(size)*count)/size);
}
static int Write(KdObject*,const void* b,unsigned size,unsigned count,int h)
{
    if(!size || count>UINT_MAX/size) return 0;
    return static_cast<int>(SDL_WriteIO(Handle(h)->io,b,size_t(size)*count)/size);
}
static int Getc(KdObject*,int h) {unsigned char c;return SDL_ReadIO(Handle(h)->io,&c,1)==1 ? c : EOF;}
static int Putc(KdObject*,int c,int h) {unsigned char b=c;return SDL_WriteIO(Handle(h)->io,&b,1)==1 ? b : EOF;}
static char* Gets(KdObject* o,char* b,int size,int h)
{
    if(size<=0) return nullptr;int i=0;
    while(i<size-1) {int c=Getc(o,h);if(c==EOF) break;b[i++]=char(c);if(c=='\n') break;}
    b[i]=0;return i ? b : nullptr;
}
static int Eof(KdObject*,int h) {return SDL_GetIOStatus(Handle(h)->io)==SDL_IO_STATUS_EOF;}
static int Error(KdObject*,int h) {return SDL_GetIOStatus(Handle(h)->io)==SDL_IO_STATUS_ERROR;}
static void Clear(KdObject*,int h) {SDL_SeekIO(Handle(h)->io,0,SDL_IO_SEEK_CUR);}
static int Seek(KdObject*,int h,long long offset,int origin) {return SDL_SeekIO(Handle(h)->io,offset,static_cast<SDL_IOWhence>(origin))<0 ? -1 : 0;}
static long long Tell(KdObject*,int h) {return SDL_TellIO(Handle(h)->io);}
static int Stat(KdObject*,const char* path,KdFileStat* out)
{
    char translated[260];path=CopyExeDir(path,translated);SDL_PathInfo info;
    if(!path || !SDL_GetPathInfo(path,&info)) return -1;
    memset(out,0,sizeof(*out));out->size=info.size;
    out->mtime=info.modify_time/1000000000;out->mode=info.type==SDL_PATHTYPE_DIRECTORY ? 0040000 : 0100000;
    return 0;
}
static int Fstat(KdObject* o,int h,KdFileStat* out) {return Stat(o,Handle(h)->path.c_str(),out);}
static int Access(KdObject* o,const char* path,int) {KdFileStat out;return Stat(o,path,&out);}
static void* Map(KdObject*,int h,unsigned* size)
{
    File* f=Handle(h);Sint64 length=SDL_GetIOSize(f->io);if(length<0 || length>UINT_MAX) return nullptr;
    void* data=malloc(length ? size_t(length) : 1);if(!data) return nullptr;
    Sint64 position=SDL_TellIO(f->io);SDL_SeekIO(f->io,0,SDL_IO_SEEK_SET);
    size_t got=SDL_ReadIO(f->io,data,size_t(length));SDL_SeekIO(f->io,position,SDL_IO_SEEK_SET);
    if(got!=size_t(length)) {free(data);return nullptr;}*size=unsigned(length);return data;
}
static void Unmap(KdObject*,int,void* view) {free(view);}
static int ConstructFile(KdObject* o)
{
    o->scheme="file";o->destructor=Destroy;o->open=Open;o->close=Close;o->flush=Flush;
    o->read=Read;o->write=Write;o->getc=Getc;o->putc=Putc;o->gets=Gets;o->eof=Eof;o->error=Error;
    o->clearerr=Clear;o->seek=Seek;o->tell=Tell;o->stat=Stat;o->fstat=Fstat;o->access=Access;o->map=Map;o->unmap=Unmap;
    char path[260];const char* dir=CopyExeDir("data/",path);if(dir) SDL_CreateDirectory(dir);return 0;
}
void InitKdFilesystem() {CreateKdObject(ConstructFile,nullptr);}
struct Directory {char* current;char** names;int count,index;};
unsigned char* DirOpen()
{
    Directory* d=static_cast<Directory*>(calloc(1,sizeof(Directory)));
    d->names=SDL_GlobDirectory(".",nullptr,0,&d->count);
    if(!d->names) {free(d);return nullptr;}return reinterpret_cast<unsigned char*>(d);
}
void* DirRead(unsigned char* dir)
{
    Directory* d=reinterpret_cast<Directory*>(dir);
    if(!d) return nullptr;
    if(d->index==d->count) {SDL_free(d->names);d->names=nullptr;return nullptr;}
    d->current=d->names[d->index++];return &d->current;
}
int KdRename(const char* a,const char* b)
{
    char pathA[260],pathB[260];a=CopyExeDir(a,pathA);b=CopyExeDir(b,pathB);
    return a && b && SDL_RenamePath(a,b) ? 0 : -1;
}
}
