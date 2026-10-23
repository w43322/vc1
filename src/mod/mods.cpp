#include "mod/mods.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace vc1 {
namespace {
struct Json {
    bool object;
    std::string text;
    std::map<std::string, Json> members;
    Json() : object(false) {}
};
// Manifests deliberately accept only objects and strings. Bound input size,
// nesting and duplicate keys so malformed metadata cannot partially apply.
class Parser {
    const char* p;
    const char* end;
    void Space() { while(p!=end && (*p==' ' || *p=='\r' || *p=='\n' || *p=='\t')) ++p; }
    bool String(std::string& out) {
        Space();if(p==end || *p++!='"') return false;
        while(p!=end) {
            unsigned char c=*p++;
            if(c=='"') return true;
            if(c<32) return false;
            if(c!='\\') {out+=c;continue;}
            if(p==end) return false;
            c=*p++;
            switch(c) {
            case '"':case '\\':case '/':out+=c;break;
            case 'b':out+='\b';break;case 'f':out+='\f';break;
            case 'n':out+='\n';break;case 'r':out+='\r';break;case 't':out+='\t';break;
            case 'u': {
                unsigned code=0;
                for(int i=0;i<4;++i) {
                    if(p==end) return false;
                    c=*p++;unsigned digit=c>='0' && c<='9' ? c-'0' :
                        c>='a' && c<='f' ? c-'a'+10 : c>='A' && c<='F' ? c-'A'+10 : 16;
                    if(digit==16) return false;code=code*16+digit;
                }
                // Non-BMP text can be written directly as UTF-8 instead.
                if(code==0 || (code>=0xd800 && code<=0xdfff)) return false;
                if(code<128) out+=char(code);
                else if(code<2048) {out+=char(0xc0|(code>>6));out+=char(0x80|(code&63));}
                else {out+=char(0xe0|(code>>12));out+=char(0x80|((code>>6)&63));out+=char(0x80|(code&63));}
                break;
            }
            default:return false;
            }
        }
        return false;
    }
    bool Value(Json& value, int depth) {
        Space();if(p==end || depth>8) return false;
        if(*p=='"') return String(value.text);
        if(*p++!='{') return false;
        value.object=true;Space();if(p!=end && *p=='}') {++p;return true;}
        for(;;) {
            std::string key;Json child;
            if(!String(key)) return false;
            Space();if(p==end || *p++!=':' || !Value(child,depth+1)) return false;
            if(!value.members.insert(std::make_pair(key,child)).second) return false;
            Space();if(p==end) return false;
            if(*p=='}') {++p;return true;}
            if(*p++!=',') return false;
        }
    }
public:
    Parser(const unsigned char* bytes, unsigned size) : p(reinterpret_cast<const char*>(bytes)),end(p+size) {
        if(size>=3 && static_cast<unsigned char>(p[0])==0xef &&
            static_cast<unsigned char>(p[1])==0xbb && static_cast<unsigned char>(p[2])==0xbf) p+=3;
    }
    bool Read(Json& value) {if(!Value(value,0)) return false;Space();return p==end && value.object;}
};
static ModHost s_host;
struct Fragment {std::string atlas;int u,v,x,y,width,height;};
struct Patch {
    std::string mod, tile, image;
    bool resolved, failed;
    int width,height;
    std::vector<Fragment> fragments;
    Patch() : resolved(false),failed(false),width(0),height(0) {}
};
static std::vector<Patch> s_patches;
static std::vector<unsigned char> s_tiles;
static bool s_tileRead;
static void Log(const std::string& text) {if(s_host.log) s_host.log(text.c_str());}
static std::string Trim(const std::string& s) {
    size_t a=s.find_first_not_of(" \t\r\n");
    return a==std::string::npos ? "" : s.substr(a,s.find_last_not_of(" \t\r\n")-a+1);
}
static std::string Path(std::string name) {
    std::replace(name.begin(),name.end(),'\\','/');
    while(name.compare(0,2,"./")==0) name.erase(0,2);
    for(size_t i=0;i<name.size();++i) if(name[i]>='A' && name[i]<='Z') name[i]+='a'-'A';
    return name;
}
static bool Relative(const std::string& path) {
    if(path.empty() || path[0]=='/' || path[0]=='\\' || path.find(':')!=std::string::npos ||
        path.find('\0')!=std::string::npos) return false;
    std::string normalized=Path(path);
    size_t start=0;
    while(start<normalized.size()) {
        size_t end=normalized.find('/',start);
        std::string part=normalized.substr(start,end==std::string::npos ? end : end-start);
        if(part=="..") return false;
        if(end==std::string::npos) break;start=end+1;
    }
    return true;
}
static bool Read(const std::string& path, std::vector<unsigned char>& bytes, unsigned limit) {
    unsigned char* data=0;unsigned size=0;
    if(!s_host.readFile(path.c_str(),&data,&size)) return false;
    bool ok=data && size<=limit;
    if(ok) bytes.assign(data,data+size);
    free(data);return ok;
}
static unsigned U16(size_t at) {return s_tiles[at]|(unsigned(s_tiles[at+1])<<8);}
static int I16(size_t at) {return static_cast<short>(U16(at));}
static unsigned U32(size_t at) {return U16(at)|(U16(at+2)<<16);}
static bool Record(int tile,int* x,int* y,int* w,int* h,unsigned* start,unsigned* end) {
    if(s_tiles.size()<2) return false;
    unsigned count=U16(0);
    if(tile<0 || unsigned(tile)>=count || 2+size_t(count)*12>s_tiles.size()) return false;
    size_t at=2+size_t(tile)*12;
    *x=I16(at);*y=I16(at+2);*w=I16(at+4);*h=I16(at+6);
    *start=tile ? U32(at-4) : 0;*end=U32(at+8);
    return *w>0 && *h>0 && *start<=*end && *end<=(s_tiles.size()-2-count*12)/2;
}
static bool Fragments(int tile,int ox,int oy,Patch& patch,int depth) {
    if(depth>16 || patch.fragments.size()>256) return false;
    int x,y,w,h;unsigned start,end;
    if(!Record(tile,&x,&y,&w,&h,&start,&end)) return false;
    size_t base=2+U16(0)*12;
    int penX=0,penY=0;
    while(start<end) {
        unsigned command=U16(base+start++*2),op=command&63;
        if(op==0) {
            if(end-start<2) return false;
            Fragment f;char name[512];
            if(!s_host.atlasName(((command&192)<<2)|(command>>8),name,sizeof(name))) return false;
            f.atlas=Path(name);f.u=I16(base+start++*2);f.v=I16(base+start++*2);
            f.x=ox+x;f.y=oy+y;f.width=w;f.height=h;
            patch.fragments.push_back(f);
        } else if(op==3) {
            if(end-start<2) return false;
            penX=I16(base+start++*2);penY=I16(base+start++*2);
        } else if(op==4) {
            if(start==end) return false;
            if(!Fragments(U16(base+start++*2),ox+penX,oy+penY,patch,depth+1)) return false;
        } else return false; // Animated, flipped or procedural tiles need a different adapter.
    }
    return true;
}
static bool Resolve(Patch& patch) {
    int tile,x,y,w,h;unsigned start,end;
    if(!s_host.tileId(patch.tile.c_str(),&tile)) return false;
    if(!s_tileRead) {
        s_tileRead=true;
        if(!Read("res/Tiles.bin",s_tiles,16*1024*1024)) return false;
    }
    if(!Record(tile,&x,&y,&w,&h,&start,&end) || w>4096 || h>4096 ||
        !Fragments(tile,-x,-y,patch,0) || patch.fragments.empty()) return false;
    patch.width=w;patch.height=h;
    for(size_t i=0;i<patch.fragments.size();++i) {
        const Fragment& f=patch.fragments[i];
        if(f.x<0 || f.y<0 || f.x+f.width>w || f.y+f.height>h) return false;
    }
    return true;
}
// Filter in premultiplied alpha so transparent PNG edges do not acquire dark halos.
static unsigned Sample(const ModImage& image,double x,double y,double width,double height) {
    if(x<0 || y<0 || x>=width || y>=height) return 0;
    double sx=(x+0.5)*image.width/width-0.5,sy=(y+0.5)*image.height/height-0.5;
    int ix=int(std::floor(sx)),iy=int(std::floor(sy));double fx=sx-ix,fy=sy-iy;
    double a=0,r=0,g=0,b=0;
    for(int row=0;row<2;++row) for(int col=0;col<2;++col) {
        unsigned px=unsigned((std::max)(0,(std::min)(int(image.width)-1,ix+col)));
        unsigned py=unsigned((std::max)(0,(std::min)(int(image.height)-1,iy+row)));
        unsigned p=image.pixels[py*image.width+px];double weight=(col ? fx : 1-fx)*(row ? fy : 1-fy);
        double alpha=(p>>24)*weight;a+=alpha;r+=((p>>16)&255)*alpha;g+=((p>>8)&255)*alpha;b+=(p&255)*alpha;
    }
    if(a<0.5) return 0;
    return (unsigned(a+0.5)<<24)|(unsigned(r/a+0.5)<<16)|(unsigned(g/a+0.5)<<8)|unsigned(b/a+0.5);
}
}
void ShutdownMods() {s_patches.clear();s_tiles.clear();s_tileRead=false;}
void InitMods(const char* directory,const ModHost& host) {
    ShutdownMods();s_host=host;
    std::string root=directory;std::vector<unsigned char> bytes;
    if(!Read(root+"/enabled.txt",bytes,65536)) {Log("No enabled.txt; original resources selected");return;}
    std::string list(bytes.begin(),bytes.end());size_t pos=0;std::set<std::string> seen;
    while(pos<list.size()) {
        size_t end=list.find('\n',pos);std::string folder=Trim(list.substr(pos,end==std::string::npos ? end : end-pos));
        pos=end==std::string::npos ? list.size() : end+1;
        if(folder.empty() || folder[0]=='#') continue;
        if(folder.find_first_of("/\\:")!=std::string::npos || folder=="." || folder==".." ||
            !Relative(folder) || !seen.insert(folder).second) {Log("Invalid or repeated mod folder: "+folder);continue;}
        std::string base=root+"/"+folder;
        if(!Read(base+"/mod.json",bytes,65536)) {Log("Cannot read mod: "+folder);continue;}
        Json manifest;
        if(bytes.empty() || !Parser(&bytes[0],unsigned(bytes.size())).Read(manifest)) {Log("Invalid manifest: "+folder);continue;}
        std::map<std::string,Json>::const_iterator tiles=manifest.members.find("tiles"),id=manifest.members.find("id"),
            version=manifest.members.find("version");
        if(tiles==manifest.members.end() || !tiles->second.object || id==manifest.members.end() ||
            id->second.object || id->second.text!=folder || version==manifest.members.end() || version->second.object) {
            Log("Expected matching id, version and tiles: "+folder);continue;
        }
        const Json& tileMap=tiles->second;
        bool valid=true;
        for(std::map<std::string,Json>::const_iterator i=tileMap.members.begin();i!=tileMap.members.end();++i)
            if(i->first.empty() || i->second.object || !Relative(i->second.text)) valid=false;
        if(!valid) {Log("Invalid tile replacement path: "+folder);continue;}
        for(std::map<std::string,Json>::const_iterator i=tileMap.members.begin();i!=tileMap.members.end();++i) {
            Patch patch;patch.mod=folder;patch.tile=i->first;patch.image=base+"/"+i->second.text;
            for(size_t n=0;n<s_patches.size();++n) if(s_patches[n].tile==patch.tile)
                Log(folder+" overrides "+s_patches[n].mod+": "+patch.tile);
            s_patches.push_back(patch);
        }
        Log("Enabled "+folder+" "+version->second.text);
    }
}
bool ApplyTextureMods(const char* name,unsigned width,unsigned height,unsigned* pixels) {
    bool changed=false;std::string path=Path(name);
    for(size_t i=0;i<s_patches.size();++i) {
        Patch& patch=s_patches[i];if(patch.failed) continue;
        if(!patch.resolved) {
            int tile;
            // Splash textures can load before Tiles.nut has populated the VM.
            if(!s_host.tileId(patch.tile.c_str(),&tile)) continue;
            patch.resolved=true;
            if(!Resolve(patch)) {patch.failed=true;Log("Unsupported or missing tile: "+patch.tile);continue;}
        }
        bool applies=false,valid=true;
        for(size_t n=0;n<patch.fragments.size();++n) {
            const Fragment& f=patch.fragments[n];if(f.atlas!=path) continue;
            applies=true;
            if(f.u<0 || f.v<0 || unsigned(f.u+f.width)>width || unsigned(f.v+f.height)>height) valid=false;
        }
        if(!applies) continue;
        ModImage image={0,0,0};
        if(!valid || !s_host.readPng(patch.image.c_str(),&image) || !image.pixels ||
            !image.width || !image.height || image.width>8192 || image.height>8192) {
            free(image.pixels);patch.failed=true;Log("Cannot apply PNG: "+patch.image);continue;
        }
        // Fill the original logical canvas. Individual UI controls may use
        // only a sub-tile; preserve that original crop rather than shrinking
        // every use of a shared atlas region to fit the narrowest control.
        for(size_t n=0;n<patch.fragments.size();++n) {
            const Fragment& f=patch.fragments[n];if(f.atlas!=path) continue;
            for(int y=0;y<f.height;++y) for(int x=0;x<f.width;++x)
                pixels[(f.v+y)*width+f.u+x]=Sample(image,f.x+x,f.y+y,patch.width,patch.height);
        }
        free(image.pixels);changed=true;Log("Applied "+patch.mod+": "+patch.tile+" -> "+path);
    }
    return changed;
}
}
