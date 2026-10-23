#include "mod/mods.h"
#include "platform/sdl3/runtime.h"
#include "sqplus/SquirrelVM.h"
#include <SDL3/SDL.h>
#include <squirrel.h>
#include <cstdlib>
#include <cstring>

// Keep SqPlus's std::string alias out of the portable mod core.
namespace vc1 {
namespace {
bool ReadFile(const char* name,unsigned char** bytes,unsigned* length) {
    size_t size=0;void* data=SDL_LoadFile(name,&size);
    if(!data || size>16*1024*1024) {SDL_free(data);return false;}
    *bytes=static_cast<unsigned char*>(malloc(size ? size : 1));
    if(!*bytes) {SDL_free(data);return false;}
    memcpy(*bytes,data,size);*length=unsigned(size);SDL_free(data);return true;
}
bool ReadPng(const char* name,ModImage* image) {
    SDL_Surface* surface=SDL_LoadPNG(name);
    if(!surface) return false;
    bool ok=surface->w>0 && surface->h>0 && surface->w<=8192 && surface->h<=8192;
    if(ok) {
        image->width=surface->w;image->height=surface->h;
        image->pixels=static_cast<unsigned*>(malloc(size_t(surface->w)*surface->h*4));
        ok=image->pixels && SDL_ConvertPixels(surface->w,surface->h,surface->format,
            surface->pixels,surface->pitch,SDL_PIXELFORMAT_ARGB8888,image->pixels,surface->w*4);
    }
    SDL_DestroySurface(surface);return ok;
}
bool TileId(const char* name,int* tile) {
    HSQUIRRELVM vm=SquirrelVM::_VM;if(!vm) return false;
    SQInteger top=sq_gettop(vm),id;bool ok=false;
    sq_pushroottable(vm);sq_pushstring(vm,"Tiles",-1);
    if(SQ_SUCCEEDED(sq_rawget(vm,-2))) {
        sq_pushstring(vm,name,-1);
        ok=SQ_SUCCEEDED(sq_rawget(vm,-2)) && SQ_SUCCEEDED(sq_getinteger(vm,-1,&id));
        if(ok) *tile=id;
    }
    sq_settop(vm,top);return ok;
}
bool AtlasName(int index,char* name,unsigned capacity) {
    HSQUIRRELVM vm=SquirrelVM::_VM;if(!vm) return false;
    SQInteger top=sq_gettop(vm);bool ok=false;const SQChar* value=0;
    sq_pushroottable(vm);sq_pushstring(vm,"TileImages",-1);
    if(SQ_SUCCEEDED(sq_rawget(vm,-2))) {
        sq_pushinteger(vm,index);
        if(SQ_SUCCEEDED(sq_get(vm,-2)) && SQ_SUCCEEDED(sq_getstring(vm,-1,&value)) &&
            strlen(value)+5<capacity) {SDL_snprintf(name,capacity,"res/%s",value);ok=true;}
    }
    sq_settop(vm,top);return ok;
}
void Log(const char* message) {SDL_Log("[Mods] %s",message);}
}
void InitSdlMods() {
    ModHost host={ReadFile,ReadPng,TileId,AtlasName,Log};
    const char* overrideRoot=SDL_getenv("VC1_MODS");
    char* root=overrideRoot ? SDL_strdup(overrideRoot) : nullptr;
    if(!overrideRoot && SDL_GetBasePath()) SDL_asprintf(&root,"%smods",SDL_GetBasePath());
    if(root) {InitMods(root,host);SDL_free(root);}
}
}
