// Included in display.cpp's vc1 namespace. SDL owns all rendering resources.
// Native handles in the engine are opaque storage, never COM objects here.
SDL_Window* g_sdlWindow;
SDL_Renderer* g_sdlRenderer;
static BatchVertex s_vertices[4000];
static unsigned s_flags;
static IUnknown* s_texture;
static Matrix3 s_transform;
static bool s_drawing;
static KdRenderTargetNode* s_target;
static KdRenderTargetNode* s_targets;
static int s_mode[3];

struct SdlTexture {
    SDL_Texture* texture;
    unsigned width, height;
    std::vector<unsigned> pixels;
    std::vector<unsigned> lock;
    SDL_Rect region;
    bool writable, target;
};
static SdlTexture* Native(IUnknown* p) { return reinterpret_cast<SdlTexture*>(p); }
unsigned SdlTextureWidth(IUnknown* p) { return p ? Native(p)->width : 0; }
unsigned SdlTextureHeight(IUnknown* p) { return p ? Native(p)->height : 0; }
unsigned long SdlReleaseTexture(IUnknown* p)
{
    if (!p) return 0;
    if(s_texture==p) {FlushBatch(0);s_texture=nullptr;}
    if(s_target && s_target->texture==p) SdlBeginTarget(nullptr);
    SDL_DestroyTexture(Native(p)->texture);
    delete Native(p);
    return 0;
}
static IUnknown* NewTexture(unsigned w, unsigned h, bool target)
{
    if (!w || !h || w > 16384 || h > 16384) return 0;
    SdlTexture* p = new SdlTexture();
    p->width = w; p->height = h; p->target = target; p->writable = false;
    p->pixels.resize(static_cast<size_t>(w) * h);
    p->texture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888,
        target ? SDL_TEXTUREACCESS_TARGET : SDL_TEXTUREACCESS_STATIC, w, h);
    if (!p->texture) { delete p; return 0; }
    SDL_SetTextureScaleMode(p->texture, SDL_SCALEMODE_LINEAR);
    if (!target) SDL_UpdateTexture(p->texture, 0, p->pixels.data(), w * 4);
    return reinterpret_cast<IUnknown*>(p);
}
IUnknown* CreateManagedTexture(unsigned width, unsigned height)
{
    return NewTexture(width, height, false);
}
void* LockTextureRect(IUnknown* p, int x, int y, bool readOnly, int w, int h)
{
    if (!p) return 0;
    FlushBatch(0);
    SdlTexture* t = Native(p);
    if (!t->lock.empty()) return 0;
    if (!w || !h) { x = y = 0; w = t->width; h = t->height; }
    if (x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > int(t->width) || y + h > int(t->height)) return 0;
    if (t->target) {
        SDL_Texture* old = SDL_GetRenderTarget(g_sdlRenderer);
        SDL_SetRenderTarget(g_sdlRenderer, t->texture);
        SDL_Surface* image = SDL_RenderReadPixels(g_sdlRenderer, 0);
        SDL_SetRenderTarget(g_sdlRenderer, old);
        if (!image) return 0;
        bool ok = SDL_ConvertPixels(t->width,t->height,image->format,image->pixels,
            image->pitch,SDL_PIXELFORMAT_ARGB8888,t->pixels.data(),t->width*4);
        SDL_DestroySurface(image);
        if (!ok) return 0;
    }
    t->region = {x,y,w,h}; t->writable = !readOnly;
    t->lock.resize(static_cast<size_t>(w)*h);
    for (int row=0; row<h; ++row)
        memcpy(t->lock.data()+row*w,t->pixels.data()+(y+row)*t->width+x,w*4);
    return t->lock.data();
}
void SdlUnlockTexture(IUnknown* p)
{
    if (!p || Native(p)->lock.empty()) return;
    SdlTexture* t = Native(p);
    if (t->writable) {
        const SDL_Rect& r = t->region;
        for (int row=0; row<r.h; ++row)
            memcpy(t->pixels.data()+(r.y+row)*t->width+r.x,t->lock.data()+row*r.w,r.w*4);
        SDL_UpdateTexture(t->texture,&r,t->lock.data(),r.w*4);
    }
    t->lock.clear();
}
static SDL_FColor Color(unsigned c)
{
    return SDL_FColor{((c>>16)&255)/255.f,((c>>8)&255)/255.f,(c&255)/255.f,(c>>24)/255.f};
}
static SDL_FPoint Position(const BatchVertex& v)
{
    return SDL_FPoint{v.x*s_transform.m[0]+v.y*s_transform.m[3]+s_transform.m[6],
        v.x*s_transform.m[1]+v.y*s_transform.m[4]+s_transform.m[7]};
}
static SDL_FColor TextureColor(const BatchVertex* vertices,float u,float v)
{
    int corners[3]={0,1,2};float weights[3]={1,0,0};
    for(int triangle=0;triangle<2;++triangle) {
        if(triangle) {corners[1]=2;corners[2]=3;}
        const BatchVertex& a=vertices[corners[0]];
        const BatchVertex& b=vertices[corners[1]];
        const BatchVertex& c=vertices[corners[2]];
        float d=(b.v-c.v)*(a.u-c.u)+(c.u-b.u)*(a.v-c.v);
        if(std::fabs(d)<0.000001f) continue;
        weights[0]=((b.v-c.v)*(u-c.u)+(c.u-b.u)*(v-c.v))/d;
        weights[1]=((c.v-a.v)*(u-c.u)+(a.u-c.u)*(v-c.v))/d;
        weights[2]=1-weights[0]-weights[1];
        if(weights[0]>=-0.001f && weights[1]>=-0.001f && weights[2]>=-0.001f) break;
    }
    SDL_FColor result={0,0,0,0};
    for(int i=0;i<3;++i) {
        SDL_FColor c=Color(vertices[corners[i]].color);
        result.r+=c.r*weights[i];result.g+=c.g*weights[i];result.b+=c.b*weights[i];result.a+=c.a*weights[i];
    }
    return result;
}
static SDL_Texture* AddColorTexture(SdlTexture* source,const BatchVertex* vertices)
{
    // D3DTOP_ADD saturates texture + diffuse before alpha blending. SDL's
    // standard geometry shader only multiplies; shade this uncommon mode into
    // a temporary texture so it works with software and accelerated renderers.
    if(source->target) {
        SDL_Texture* old=SDL_GetRenderTarget(g_sdlRenderer);
        SDL_SetRenderTarget(g_sdlRenderer,source->texture);
        SDL_Surface* image=SDL_RenderReadPixels(g_sdlRenderer,nullptr);
        SDL_SetRenderTarget(g_sdlRenderer,old);
        if(!image) return nullptr;
        SDL_ConvertPixels(source->width,source->height,image->format,image->pixels,image->pitch,
            SDL_PIXELFORMAT_ARGB8888,source->pixels.data(),source->width*4);
        SDL_DestroySurface(image);
    }
    std::vector<unsigned> pixels(source->pixels.size());
    for(unsigned y=0;y<source->height;++y) for(unsigned x=0;x<source->width;++x) {
        SDL_FColor tint=TextureColor(vertices,(x+0.5f)/source->width,(y+0.5f)/source->height);
        unsigned p=source->pixels[y*source->width+x];
        auto byte=[](float value){return unsigned((std::max)(0.f,(std::min)(255.f,value))+0.5f);};
        unsigned r=byte(((p>>16)&255)+tint.r*255),g=byte(((p>>8)&255)+tint.g*255),b=byte((p&255)+tint.b*255);
        unsigned a=byte((p>>24)*tint.a);pixels[y*source->width+x]=(a<<24)|(r<<16)|(g<<8)|b;
    }
    SDL_Texture* result=SDL_CreateTexture(g_sdlRenderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STATIC,source->width,source->height);
    if(result) {SDL_UpdateTexture(result,nullptr,pixels.data(),source->width*4);SDL_SetTextureScaleMode(result,SDL_SCALEMODE_LINEAR);}
    return result;
}
static void FlushBatch(char)
{
    if (!g_batchCount || !g_sdlRenderer) return;
    std::vector<SDL_Vertex> vertices(g_batchCount*4);
    std::vector<int> indices(g_batchCount*6);
    for (int q=0; q<g_batchCount; ++q) {
        for(int i=0;i<4;++i) {
            const BatchVertex& v=s_vertices[q*4+i];
            vertices[q*4+i]={Position(v),Color(v.color),{v.u,v.v}};
        }
        const int order[6]={0,1,2,0,2,3};
        for(int i=0;i<6;++i) indices[q*6+i]=q*4+order[i];
    }
    SDL_BlendMode mode=(s_flags&2) ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD;
    SDL_Texture* texture=s_texture ? Native(s_texture)->texture : 0;
    if(texture) SDL_SetTextureBlendMode(texture,mode);
    else SDL_SetRenderDrawBlendMode(g_sdlRenderer,mode);
    if(texture && (s_flags&1)) {
        const int order[6]={0,1,2,0,2,3};
        for(int q=0;q<g_batchCount;++q) {
            SDL_Texture* shaded=AddColorTexture(Native(s_texture),s_vertices+q*4);
            if(!shaded) {SDL_Log("SDL texture color operation: %s",SDL_GetError());continue;}
            SDL_SetTextureBlendMode(shaded,mode);
            SDL_Vertex quad[4];
            for(int i=0;i<4;++i) {quad[i]=vertices[q*4+i];quad[i].color={1,1,1,1};}
            SDL_RenderGeometry(g_sdlRenderer,shaded,quad,4,order,6);SDL_DestroyTexture(shaded);
        }
        g_batchCount=0;return;
    }
    if(!SDL_RenderGeometry(g_sdlRenderer,texture,vertices.data(),vertices.size(),indices.data(),indices.size()))
        SDL_Log("SDL geometry: %s",SDL_GetError());
    g_batchCount=0;
}
void* BeginQuadBatch(unsigned flags,IUnknown* texture,int* capacity)
{
    if(!s_drawing) return 0;
    FlushBatch(0); s_flags=flags; s_texture=texture; *capacity=1000;
    return s_vertices;
}
void PushQuad(const BatchQuad* quad)
{
    if(!s_drawing) return;
    if(g_batchCount==1000 || s_texture!=quad->texture || s_flags!=quad->flags) FlushBatch(0);
    s_texture=quad->texture; s_flags=quad->flags;
    memcpy(s_vertices+g_batchCount*4,quad->vertices,sizeof(quad->vertices)); ++g_batchCount;
}
void BatchLine(float x0,float y0,float x1,float y1,float z,unsigned color)
{
    if(!s_drawing) return;
    FlushBatch(0);
    BatchVertex a={x0,y0,z,color,0,0}, b={x1,y1,z,color,0,0};
    SDL_FPoint from=Position(a),to=Position(b);
    SDL_FColor c=Color(color);
    SDL_SetRenderDrawBlendMode(g_sdlRenderer,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColorFloat(g_sdlRenderer,c.r,c.g,c.b,c.a);
    SDL_RenderLine(g_sdlRenderer,from.x,from.y,to.x,to.y);
}
void DisplayD3dSetView(const Matrix3* matrix) { FlushBatch(0); s_transform=*matrix; }
void DisplayD3dSetClip(int x,int y,int width,int height)
{
    FlushBatch(0);
    SDL_Rect r={x,y,(std::max)(0,width),(std::max)(0,height)};
    SDL_SetRenderClipRect(g_sdlRenderer,&r);
}
void SdlClear(unsigned color)
{
    FlushBatch(0);
    SDL_FColor c=Color(color);
    SDL_SetRenderDrawColorFloat(g_sdlRenderer,c.r,c.g,c.b,c.a);
    SDL_RenderClear(g_sdlRenderer);
}
void* LookupRenderTargetTexture(unsigned width,unsigned height)
{
    IUnknown* texture=NewTexture(width,height,true);
    if(!texture) return 0;
    KdRenderTargetNode* node=static_cast<KdRenderTargetNode*>(calloc(1,sizeof(KdRenderTargetNode)));
    node->width=width; node->height=height; node->texture=texture; node->next=s_targets;
    s_targets=node; return node;
}
bool SdlBeginTarget(KdRenderTargetNode* node)
{
    if(!g_sdlRenderer) return 0;
    FlushBatch(0);
    SDL_SetRenderTarget(g_sdlRenderer,node ? Native(node->texture)->texture : 0);
    SDL_SetRenderLogicalPresentation(g_sdlRenderer,node ? node->width : g_displayWidth,
        node ? node->height : g_displayHeight,node ? SDL_LOGICAL_PRESENTATION_DISABLED : SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetRenderViewport(g_sdlRenderer,0);
    SDL_SetRenderClipRect(g_sdlRenderer,0);
    s_target=node; s_transform=Matrix3::Identity; s_drawing=true;
    return 1;
}
static unsigned char DisplayQueryReady(KdRenderTargetNode* node) { return SdlBeginTarget(node); }
void SdlEndFrame()
{
    FlushBatch(1); s_drawing=false;
    if(!s_target) SDL_RenderPresent(g_sdlRenderer);
}
void* IRenderTarget::EndFrame() { static_cast<KdGraphics*>(graphics)->BindScene(); SdlEndFrame(); return 0; }
void* KdDisplayBase::EndFrame() { return IRenderTarget::EndFrame(); }
void* KdDisplay::EndFrame() { return KdDisplayBase::EndFrame(); }
int KdDisplayBase::FitWindow() { SDL_SetWindowPosition(g_sdlWindow,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED); return 1; }
int KdDisplay::FitWindow() { enabled58=1; return KdDisplayBase::FitWindow(); }
static int ApplyWindowedMode(bool windowed)
{
    if(!g_sdlWindow) return 0;
    FlushBatch(0);
    if(!SDL_SetWindowFullscreen(g_sdlWindow,!windowed)) return 0;
    g_displaySkipDevice=windowed;
    return 1;
}
unsigned char KdDisplayBase::SetSize(unsigned width,unsigned height)
{
    HostLog(&g_kdNameDisplay,"Init: %dx%d",width,height);
    if(!SdlCreateDisplay(width,height)) { HostLogError(&g_kdNameDisplay,"SDL display: %s",SDL_GetError()); return 0; }
    mixer=g_kdWindowMixer;
    g_displayWidth=width; g_displayHeight=height;
    s_mode[0]=width;s_mode[1]=height;s_mode[2]=0x15;g_displayMode=s_mode;
    g_displayMatrixInv=Matrix3::Identity;
    KdInstallCallback(DisplayEventCallback,0,mixer);
    ApplyWindowedMode(GetKdEnv("VC1_WINDOWED") != 0);
    return KdDisplayDevice::SetSize(width,height);
}
unsigned char KdDisplay::SetSize(unsigned width,unsigned height) { return KdDisplayBase::SetSize(width,height); }
int KdDisplayBase::Shutdown() throw()
{
    if(!mixer) return 0;
    KdDisplayDevice::Shutdown();
    KdInstallCallback(0,0,mixer);
    while(g_d3dChain2) ReleaseTrackedTexture(g_d3dChain2->texture);
    while(s_targets) { KdRenderTargetNode* n=s_targets->next; SdlReleaseTexture(s_targets->texture); free(s_targets); s_targets=n; }
    s_drawing=false; g_batchCount=0; s_target=0;
    SdlDestroyDisplay(); mixer=0;
    return 0;
}

static IUnknown* LoadSdlPvr(const char* name)
{
    unsigned header[13];
    void* file=OpenKdFile(name,"rb");
    if(!file) return 0;
    IUnknown* texture=0;
    if(KdFread(header,sizeof(header),1,file)==1 && header[6]==32 && header[0]>=52) {
        texture=NewTexture(header[2],header[1],false);
        if(texture) {
            SdlTexture* t=Native(texture);
            if(header[0]>52) { std::vector<unsigned char> skip(header[0]-52); KdFread(skip.data(),1,skip.size(),file); }
            if(KdFread(t->pixels.data(),4,t->pixels.size(),file)!=int(t->pixels.size())) { SdlReleaseTexture(texture);texture=0; }
            else SDL_UpdateTexture(t->texture,0,t->pixels.data(),t->width*4);
        }
    }
    KdFclose(file);return texture;
}
static IUnknown* LoadSdlTga(const char* name)
{
    TGA* tga=TGAOpen(const_cast<char*>(name),const_cast<char*>("rb"));
    if(!tga) return 0;
    IUnknown* texture=0;
    if(TGAReadHeader(tga)==TGA_OK && (tga->hdr.depth==24 || tga->hdr.depth==32)) {
        texture=NewTexture(tga->hdr.width,tga->hdr.height,false);
        if(texture) {
            SdlTexture* t=Native(texture);
            std::vector<unsigned char> row(t->width*4);
            for(unsigned y=0;y<t->height;++y) {
                if(TGAReadScanlines(tga,row.data(),y,1,0)!=1) { SdlReleaseTexture(texture);texture=0;break; }
                unsigned* dst=t->pixels.data()+(tga->hdr.vert ? y : t->height-1-y)*t->width;
                for(unsigned x=0;x<t->width;++x) {
                    unsigned char* p=row.data()+x*(tga->hdr.depth/8);
                    dst[x]=p[0]|(p[1]<<8)|(p[2]<<16)|((tga->hdr.depth==32 ? p[3] : 255u)<<24);
                }
            }
            if(texture) SDL_UpdateTexture(t->texture,0,t->pixels.data(),t->width*4);
        }
    }
    TGAClose(tga);return texture;
}
static IUnknown* LoadSdlJpg(const char* name)
{
    jpeg_decoder_file_stream stream;
    if(stream.open(name)) return 0;
    jpeg_decoder decoder(&stream,false);
    if(decoder.get_error_code() || decoder.begin()) return 0;
    IUnknown* texture=NewTexture(decoder.get_width(),decoder.get_height(),false);
    if(!texture) return 0;
    SdlTexture* t=Native(texture);
    for(unsigned y=0;y<t->height;++y) {
        void* line; uint bytes;
        if(decoder.decode(&line,&bytes)) { SdlReleaseTexture(texture);return 0; }
        const unsigned char* src=static_cast<const unsigned char*>(line);
        unsigned channels=decoder.get_bytes_per_pixel();
        for(unsigned x=0;x<t->width;++x) {
            const unsigned char* p=src+x*channels;
            t->pixels[y*t->width+x]=0xff000000u|(p[0]<<16)|(p[channels==1 ? 0 : 1]<<8)|p[channels==1 ? 0 : 2];
        }
    }
    SDL_UpdateTexture(t->texture,0,t->pixels.data(),t->width*4);
    return texture;
}
IUnknown* LoadTextureFile(const char* name)
{
    const char* ext=strrchr(name,'.');
    IUnknown* texture=0;
    if(ext && !_stricmp(ext,".pvr")) texture=LoadSdlPvr(name);
    else if(ext && !_stricmp(ext,".tga")) texture=LoadSdlTga(name);
    else if(ext && (!_stricmp(ext,".jpg") || !_stricmp(ext,".jpeg"))) texture=LoadSdlJpg(name);
    if(!texture) { HostLogError(&g_kdNameTexture,"Cannot load %s: %s",name,SDL_GetError());return 0; }
    TrackedTexture* node=static_cast<TrackedTexture*>(malloc(sizeof(TrackedTexture)));
    node->texture=texture;node->width=SdlTextureWidth(texture);node->height=SdlTextureHeight(texture);
    node->next=g_d3dChain2;g_d3dChain2=node;return texture;
}
