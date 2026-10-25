// D3D8 subset over SDL3 (see d3d8.h). Slot numbers are the IDirect3D8,
// IDirect3DDevice8, IDirect3DTexture8, IDirect3DSurface8 and
// IDirect3DVertexBuffer8/IndexBuffer8 vtable indices the recovered display
// code calls; each is named at its call site in kd/display.
#include "d3d8.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <utility>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace vc1 {


namespace {

// The device's renderer; there is one device.
SDL_Renderer* g_sdlRenderer;

const int kOk = 0;
const int kInvalidCall = static_cast<int>(0x8876086cu);
const unsigned kA8R8G8B8 = 0x15;
const unsigned kX8R8G8B8 = 0x16;
const unsigned kIndex16 = 0x65;
const unsigned kFvf = 0x142;  // XYZ | DIFFUSE | TEX1
const unsigned kLockReadOnly = 0x10;
const unsigned kUsageRenderTarget = 1;
const unsigned kClearTarget = 1;
const unsigned kRsDestBlend = 0x14;
const unsigned kBlendOne = 2;
const unsigned kTssColorOp = 1;
const unsigned kTopAdd = 7;
const unsigned kTopModulate = 4;
const unsigned kTsView = 2;
const unsigned kTsProjection = 3;
const int kLineList = 2;
const int kTriangleList = 4;

struct LockedRect
{
    int pitch;
    void* bits;
};

// D3DSURFACE_DESC.
struct SurfaceDesc
{
    unsigned format, type, usage, pool, size, multiSample, width, height;
};

// D3DVIEWPORT8.
struct Viewport
{
    unsigned x, y, width, height;
    float minZ, maxZ;
};

// D3DDISPLAYMODE.
struct DisplayMode
{
    unsigned width, height, refresh, format;
};

// One FVF 0x142 vertex.
struct Vertex
{
    float x, y, z;
    unsigned color;
    float u, v;
};

// IDirect3DVertexBuffer8 / IDirect3DIndexBuffer8.
struct SdlBuffer
{
    void** vtbl;
    unsigned refs;
    std::vector<unsigned char> data;
};

// IDirect3D8.
struct SdlD3d
{
    void** vtbl;
    unsigned refs;
};

// IDirect3DDevice8. Holds references to what is bound, as D3D does.
struct SdlDevice
{
    void** vtbl;
    unsigned refs;
    SdlTexture* backBuffer;
    SdlTexture* target;
    SdlTexture* texture;
    SdlBuffer* stream;
    SdlBuffer* indices;
    Viewport viewport;
    float view[16];
    float proj[16];
    unsigned destBlend;
    unsigned colorOp;
};

void* s_d3dVtbl[0x10];
void* s_deviceVtbl[0x61];
void* s_textureVtbl[0x13];
void* s_surfaceVtbl[0x0b];
void* s_bufferVtbl[0x0e];
SdlTexture* s_textures;  // every live texture, cleared when the renderer goes

const char* const kKinds[] = {"Direct3D", "device", "texture", "surface", "buffer"};

// The stack of an unused slot is unknown, so it never returns.
template <int Kind, int Slot>
void __stdcall Unsupported()
{
    SDL_Log("SDL D3D8: unsupported %s slot 0x%x", kKinds[Kind], Slot);
    abort();
}

template <int Kind, std::size_t... Slots>
void Fill(void** table, std::index_sequence<Slots...>)
{
    ((table[Slots] = reinterpret_cast<void*>(&Unsupported<Kind, int(Slots)>)), ...);
}

template <typename T>
void Set(void** table, int slot, T function)
{
    table[slot] = reinterpret_cast<void*>(function);
}

SDL_FColor Color(unsigned c)
{
    return SDL_FColor{((c >> 16) & 255) / 255.f, ((c >> 8) & 255) / 255.f, (c & 255) / 255.f,
        (c >> 24) / 255.f};
}

// Adapter 0's desktop mode in pixels, reported as X8R8G8B8.
void DesktopMode(DisplayMode* out)
{
    const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
    int width = 1024;
    int height = 768;
    sdl3::DesktopPixels(&width, &height);
    out->width = unsigned(width);
    out->height = unsigned(height);
    out->refresh = mode ? unsigned(mode->refresh_rate + 0.5f) : 60;
    out->format = kX8R8G8B8;
}

// ---- Textures --------------------------------------------------------------

SdlTexture* NewTexture(unsigned width, unsigned height, unsigned format, bool target)
{
    if (!g_sdlRenderer || !width || !height || width > 16384 || height > 16384 ||
        (format != kA8R8G8B8 && format != kX8R8G8B8)) {
        return 0;
    }
    // X8R8G8B8 samples with alpha 1, which only an XRGB texture gives.
    SDL_Texture* handle = SDL_CreateTexture(g_sdlRenderer,
        format == kX8R8G8B8 ? SDL_PIXELFORMAT_XRGB8888 : SDL_PIXELFORMAT_ARGB8888,
        target ? SDL_TEXTUREACCESS_TARGET : SDL_TEXTUREACCESS_STATIC, width, height);
    if (!handle) {
        SDL_Log("SDL texture %ux%u: %s", width, height, SDL_GetError());
        return 0;
    }
    SDL_SetTextureScaleMode(handle, SDL_SCALEMODE_LINEAR);
    SdlTexture* texture = new SdlTexture();
    texture->vtbl = s_textureVtbl;
    texture->refs = 1;
    texture->texture = handle;
    texture->width = width;
    texture->height = height;
    texture->format = format;
    texture->target = target;
    texture->pixels.assign(size_t(width) * height, format == kX8R8G8B8 ? 0xff000000u : 0);
    texture->region = SDL_Rect{0, 0, 0, 0};
    texture->writable = false;
    if (!target) {
        SDL_UpdateTexture(handle, 0, texture->pixels.data(), width * 4);
    }
    texture->prev = 0;
    texture->next = s_textures;
    if (s_textures) {
        s_textures->prev = texture;
    }
    s_textures = texture;
    return texture;
}

// Pulls a render target's pixels into SdlTexture::pixels.
bool ReadTarget(SdlTexture* texture)
{
    if (!texture->target) {
        return true;
    }
    if (!texture->texture) {
        return false;
    }
    SDL_Texture* old = SDL_GetRenderTarget(g_sdlRenderer);
    SDL_SetRenderTarget(g_sdlRenderer, texture->texture);
    SDL_Surface* image = SDL_RenderReadPixels(g_sdlRenderer, 0);
    SDL_SetRenderTarget(g_sdlRenderer, old);
    if (!image) {
        return false;
    }
    bool ok = SDL_ConvertPixels(texture->width, texture->height, image->format, image->pixels,
        image->pitch, SDL_PIXELFORMAT_ARGB8888, texture->pixels.data(), texture->width * 4);
    SDL_DestroySurface(image);
    return ok;
}

SdlTexture* Texture(void* p) { return static_cast<SdlTexture*>(p); }

// IUnknown slots 1/2.
unsigned long __stdcall TextureAddRef(void* self) { return ++Texture(self)->refs; }
unsigned long __stdcall TextureRelease(void* self)
{
    SdlTexture* texture = Texture(self);
    if (--texture->refs) {
        return texture->refs;
    }
    if (texture->prev) {
        texture->prev->next = texture->next;
    }
    else {
        s_textures = texture->next;
    }
    if (texture->next) {
        texture->next->prev = texture->prev;
    }
    if (texture->texture) {
        SDL_DestroyTexture(texture->texture);
    }
    delete texture;
    return 0;
}

void Hold(SdlTexture** slot, SdlTexture* texture)
{
    if (texture) {
        TextureAddRef(texture);
    }
    if (*slot) {
        TextureRelease(*slot);
    }
    *slot = texture;
}

// IDirect3DTexture8 slot 0x0e.
int __stdcall TextureGetLevelDesc(void* self, unsigned level, SurfaceDesc* desc)
{
    SdlTexture* texture = Texture(self);
    if (level || !desc) {
        return kInvalidCall;
    }
    desc->format = texture->format;
    desc->type = 1;  // D3DRTYPE_SURFACE
    desc->usage = texture->target ? kUsageRenderTarget : 0;
    desc->pool = texture->target ? 0 : 1;  // D3DPOOL_DEFAULT / MANAGED
    desc->size = texture->width * texture->height * 4;
    desc->multiSample = 0;
    desc->width = texture->width;
    desc->height = texture->height;
    return kOk;
}

SdlSurface* NewSurface(SdlTexture* owner, unsigned width, unsigned height)
{
    SdlSurface* surface = new SdlSurface();
    surface->vtbl = s_surfaceVtbl;
    surface->refs = 1;
    surface->owner = owner;
    surface->width = width;
    surface->height = height;
    if (owner) {
        TextureAddRef(owner);
    }
    else {
        surface->pixels.resize(size_t(width) * height);
    }
    return surface;
}

// Slot 0x0f. The surface holds a reference to its texture.
int __stdcall TextureGetSurfaceLevel(void* self, unsigned level, void** out)
{
    SdlTexture* texture = Texture(self);
    if (level || !out) {
        return kInvalidCall;
    }
    *out = NewSurface(texture, texture->width, texture->height);
    return kOk;
}

// Slot 0x10. Managed textures only, one lock at a time; the bits are packed
// rows of the locked region.
int __stdcall TextureLockRect(void* self, unsigned level, LockedRect* locked, const RECT* rect,
    unsigned flags)
{
    SdlTexture* texture = Texture(self);
    if (level || !locked || texture->target || !texture->lock.empty()) {
        return kInvalidCall;
    }
    SDL_Rect r = {0, 0, int(texture->width), int(texture->height)};
    if (rect) {
        r = SDL_Rect{int(rect->left), int(rect->top), int(rect->right - rect->left),
            int(rect->bottom - rect->top)};
    }
    if (r.x < 0 || r.y < 0 || r.w <= 0 || r.h <= 0 || r.x + r.w > int(texture->width) ||
        r.y + r.h > int(texture->height)) {
        return kInvalidCall;
    }
    texture->region = r;
    texture->writable = (flags & kLockReadOnly) == 0;
    texture->lock.resize(size_t(r.w) * r.h);
    for (int row = 0; row < r.h; ++row) {
        memcpy(&texture->lock[size_t(row) * r.w],
            &texture->pixels[size_t(r.y + row) * texture->width + r.x], size_t(r.w) * 4);
    }
    locked->pitch = r.w * 4;
    locked->bits = texture->lock.data();
    return kOk;
}

// Slot 0x11.
int __stdcall TextureUnlockRect(void* self, unsigned level)
{
    SdlTexture* texture = Texture(self);
    if (level || texture->lock.empty()) {
        return kInvalidCall;
    }
    const SDL_Rect& r = texture->region;
    if (texture->writable) {
        for (int row = 0; row < r.h; ++row) {
            memcpy(&texture->pixels[size_t(r.y + row) * texture->width + r.x],
                &texture->lock[size_t(row) * r.w], size_t(r.w) * 4);
        }
        SDL_UpdateTexture(texture->texture, &r, texture->lock.data(), r.w * 4);
    }
    texture->lock.clear();
    return kOk;
}

// ---- Surfaces --------------------------------------------------------------

SdlSurface* Surface(void* p) { return static_cast<SdlSurface*>(p); }

unsigned long __stdcall SurfaceAddRef(void* self) { return ++Surface(self)->refs; }
unsigned long __stdcall SurfaceRelease(void* self)
{
    SdlSurface* surface = Surface(self);
    if (--surface->refs) {
        return surface->refs;
    }
    if (surface->owner) {
        TextureRelease(surface->owner);
    }
    delete surface;
    return 0;
}

// IDirect3DSurface8 slot 9. Only image surfaces have their own memory.
int __stdcall SurfaceLockRect(void* self, LockedRect* locked, const RECT* rect, unsigned flags)
{
    (void)flags;
    SdlSurface* surface = Surface(self);
    if (!locked || surface->owner) {
        return kInvalidCall;
    }
    locked->pitch = surface->width * 4;
    locked->bits = surface->pixels.data() + (rect ? rect->top * surface->width + rect->left : 0);
    return kOk;
}

// Slot 0x0a.
int __stdcall SurfaceUnlockRect(void*) { return kOk; }

// ---- Vertex and index buffers ----------------------------------------------

SdlBuffer* Buffer(void* p) { return static_cast<SdlBuffer*>(p); }

unsigned long __stdcall BufferAddRef(void* self) { return ++Buffer(self)->refs; }
unsigned long __stdcall BufferRelease(void* self)
{
    SdlBuffer* buffer = Buffer(self);
    if (--buffer->refs) {
        return buffer->refs;
    }
    delete buffer;
    return 0;
}

void HoldBuffer(SdlBuffer** slot, SdlBuffer* buffer)
{
    if (buffer) {
        BufferAddRef(buffer);
    }
    if (*slot) {
        BufferRelease(*slot);
    }
    *slot = buffer;
}

// Slot 0x0b. Draws read the memory directly, so Unlock (0x0c) has nothing to do.
int __stdcall BufferLock(void* self, unsigned offset, unsigned size, void** out, unsigned flags)
{
    (void)flags;
    SdlBuffer* buffer = Buffer(self);
    if (!out || offset > buffer->data.size() || size > buffer->data.size() - offset) {
        return kInvalidCall;
    }
    *out = buffer->data.data() + offset;
    return kOk;
}

int __stdcall BufferUnlock(void*) { return kOk; }

SdlBuffer* NewBuffer(unsigned bytes)
{
    SdlBuffer* buffer = new SdlBuffer();
    buffer->vtbl = s_bufferVtbl;
    buffer->refs = 1;
    buffer->data.resize(bytes);
    return buffer;
}

// ---- Device ----------------------------------------------------------------

SdlDevice* Device(void* p) { return static_cast<SdlDevice*>(p); }

void Identity(float* m)
{
    for (int i = 0; i < 16; ++i) {
        m[i] = (i % 5) == 0 ? 1.0f : 0.0f;
    }
}

void FullViewport(SdlDevice* device)
{
    SdlTexture* target = device->target;
    device->viewport = Viewport{0, 0, target ? target->width : 0, target ? target->height : 0,
        0.0f, 1.0f};
}

unsigned long __stdcall DeviceAddRef(void* self) { return ++Device(self)->refs; }
unsigned long __stdcall DeviceRelease(void* self)
{
    SdlDevice* device = Device(self);
    if (--device->refs) {
        return device->refs;
    }
    Hold(&device->texture, 0);
    Hold(&device->target, 0);
    Hold(&device->backBuffer, 0);
    HoldBuffer(&device->stream, 0);
    HoldBuffer(&device->indices, 0);
    // Textures the game still holds outlive the renderer without handles.
    for (SdlTexture* texture = s_textures; texture; texture = texture->next) {
        if (texture->texture) {
            SDL_DestroyTexture(texture->texture);
        }
        texture->texture = 0;
    }
    SDL_DestroyRenderer(g_sdlRenderer);
    g_sdlRenderer = 0;
    delete device;
    return 0;
}

// Slot 3. The SDL renderer is never lost.
int __stdcall DeviceTestCooperativeLevel(void*) { return kOk; }

// Slot 7. No texture caps: sizes are not rounded to powers of two.
int __stdcall DeviceGetDeviceCaps(void*, unsigned char* caps)
{
    memset(caps, 0, 0xd4);
    return kOk;
}

bool CreateBackBuffer(SdlDevice* device, const unsigned* present)
{
    SdlTexture* backBuffer = NewTexture(present[0], present[1], present[2], true);
    if (!backBuffer) {
        return false;
    }
    Hold(&device->backBuffer, backBuffer);
    TextureRelease(backBuffer);
    Hold(&device->target, backBuffer);
    SDL_SetRenderTarget(g_sdlRenderer, backBuffer->texture);
    FullViewport(device);
    return true;
}

// Slot 0x0e. A new back buffer at the presentation size.
int __stdcall DeviceReset(void* self, const unsigned* present)
{
    return CreateBackBuffer(Device(self), present) ? kOk : kInvalidCall;
}

#ifdef __EMSCRIPTEN__
// The browser shows one frame per animation frame. Waiting for the next one
// paces the game like the original's vsync (SDL only yields with a zero
// timeout) and leaves the page's thread time for input and the devtools.
EM_ASYNC_JS(void, WaitForAnimationFrame, (), {
    await new Promise(function (resolve) { requestAnimationFrame(function () { resolve(); }); });
});
#endif

// Slot 0x0f. Scales the finished back buffer into the window; the game
// rasterizes at its own resolution, so tiles never meet at fractional pixels.
int __stdcall DevicePresent(void* self, const void*, const void*, void*, const void*)
{
    SdlDevice* device = Device(self);
    SdlTexture* backBuffer = device->backBuffer;
    if (!backBuffer || !backBuffer->texture) {
        return kInvalidCall;
    }
    SDL_SetRenderTarget(g_sdlRenderer, 0);
    SDL_SetRenderLogicalPresentation(g_sdlRenderer, backBuffer->width, backBuffer->height,
        SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetRenderClipRect(g_sdlRenderer, 0);
    SDL_SetRenderDrawColor(g_sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(g_sdlRenderer);
    SDL_SetTextureBlendMode(backBuffer->texture, SDL_BLENDMODE_NONE);
    SDL_RenderTexture(g_sdlRenderer, backBuffer->texture, 0, 0);
    SDL_RenderPresent(g_sdlRenderer);
#ifdef __EMSCRIPTEN__
    WaitForAnimationFrame();
#endif
    SDL_SetRenderTarget(g_sdlRenderer, device->target ? device->target->texture : 0);
    return kOk;
}

// Slot 0x14.
int __stdcall DeviceCreateTexture(void*, unsigned width, unsigned height, unsigned levels,
    unsigned usage, unsigned format, unsigned pool, void** out)
{
    (void)levels;
    (void)pool;
    const bool target = (usage & kUsageRenderTarget) != 0;
    SdlTexture* texture = NewTexture(width, height, format, target);
    if (!out || !texture) {
        return kInvalidCall;
    }
    *out = texture;
    return kOk;
}

// Slot 0x17.
int __stdcall DeviceCreateVertexBuffer(void*, unsigned bytes, unsigned usage, unsigned fvf,
    unsigned pool, void** out)
{
    (void)usage;
    (void)pool;
    if (!out || fvf != kFvf) {
        return kInvalidCall;
    }
    *out = NewBuffer(bytes);
    return kOk;
}

// Slot 0x18.
int __stdcall DeviceCreateIndexBuffer(void*, unsigned bytes, unsigned usage, unsigned format,
    unsigned pool, void** out)
{
    (void)usage;
    (void)pool;
    if (!out || format != kIndex16) {
        return kInvalidCall;
    }
    *out = NewBuffer(bytes);
    return kOk;
}

// Slot 0x1b.
int __stdcall DeviceCreateImageSurface(void*, unsigned width, unsigned height, unsigned format,
    void** out)
{
    if (!out || !width || !height || (format != kA8R8G8B8 && format != kX8R8G8B8)) {
        return kInvalidCall;
    }
    *out = NewSurface(0, width, height);
    return kOk;
}

// Slot 0x1c. Copies from a texture level into an image surface.
int __stdcall DeviceCopyRects(void*, void* source, const RECT* rects, unsigned count,
    void* destination, const POINT* points)
{
    SdlSurface* from = Surface(source);
    SdlSurface* to = Surface(destination);
    if (!from->owner || to->owner || !ReadTarget(from->owner)) {
        return kInvalidCall;
    }
    const RECT whole = {0, 0, LONG(from->width), LONG(from->height)};
    if (!rects) {
        rects = &whole;
        count = 1;
    }
    const std::vector<unsigned>& pixels = from->owner->pixels;
    for (unsigned i = 0; i < count; ++i) {
        const RECT& r = rects[i];
        const LONG dx = points ? points[i].x : r.left;
        const LONG dy = points ? points[i].y : r.top;
        const LONG width = r.right - r.left;
        if (r.left < 0 || r.top < 0 || width <= 0 || r.right > LONG(from->width) ||
            r.bottom > LONG(from->height) || dx < 0 || dy < 0 || dx + width > LONG(to->width) ||
            dy + r.bottom - r.top > LONG(to->height)) {
            return kInvalidCall;
        }
        for (LONG row = 0; row < r.bottom - r.top; ++row) {
            memcpy(&to->pixels[(dy + row) * to->width + dx],
                &pixels[(r.top + row) * from->width + r.left], width * 4);
        }
    }
    return kOk;
}

// Slot 0x1f. The viewport resets to the whole target.
int __stdcall DeviceSetRenderTarget(void* self, void* surface, void* depth)
{
    (void)depth;
    SdlDevice* device = Device(self);
    SdlTexture* target = surface ? Surface(surface)->owner : 0;
    if (!target || !target->target || !target->texture ||
        !SDL_SetRenderTarget(g_sdlRenderer, target->texture)) {
        return kInvalidCall;
    }
    Hold(&device->target, target);
    FullViewport(device);
    return kOk;
}

// Slot 0x20.
int __stdcall DeviceGetRenderTarget(void* self, void** out)
{
    SdlDevice* device = Device(self);
    if (!out || !device->target) {
        return kInvalidCall;
    }
    *out = NewSurface(device->target, device->target->width, device->target->height);
    return kOk;
}

// Slots 0x22/0x23. Draws are immediate.
int __stdcall DeviceBeginScene(void*) { return kOk; }
int __stdcall DeviceEndScene(void*) { return kOk; }

// Slot 0x24. Fills the viewport.
int __stdcall DeviceClear(void* self, unsigned count, const void* rects, unsigned flags,
    unsigned color, float z, unsigned stencil)
{
    (void)z;
    (void)stencil;
    SdlDevice* device = Device(self);
    if (count || rects) {
        return kInvalidCall;
    }
    if (flags & kClearTarget) {
        const Viewport& vp = device->viewport;
        const SDL_FRect area = {float(vp.x), float(vp.y), float(vp.width), float(vp.height)};
        const SDL_FColor c = Color(color);
        SDL_SetRenderClipRect(g_sdlRenderer, 0);
        SDL_SetRenderDrawBlendMode(g_sdlRenderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColorFloat(g_sdlRenderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(g_sdlRenderer, &area);
    }
    return kOk;
}

// Slot 0x25. Only the view and projection transforms are set (world stays
// identity).
int __stdcall DeviceSetTransform(void* self, unsigned state, const float* matrix)
{
    SdlDevice* device = Device(self);
    if (state == kTsView) {
        memcpy(device->view, matrix, sizeof(device->view));
    }
    else if (state == kTsProjection) {
        memcpy(device->proj, matrix, sizeof(device->proj));
    }
    return kOk;
}

// Slot 0x28.
int __stdcall DeviceSetViewport(void* self, const Viewport* viewport)
{
    Device(self)->viewport = *viewport;
    return kOk;
}

// Slot 0x32. The source blend stays SRCALPHA and Z is off, so only the
// destination blend (INVSRCALPHA or ONE) changes what is drawn.
int __stdcall DeviceSetRenderState(void* self, unsigned state, unsigned value)
{
    if (state == kRsDestBlend) {
        Device(self)->destBlend = value;
    }
    return kOk;
}

// Slot 0x3d.
int __stdcall DeviceSetTexture(void* self, unsigned stage, void* texture)
{
    if (stage) {
        return kInvalidCall;
    }
    Hold(&Device(self)->texture, Texture(texture));
    return kOk;
}

// Slot 0x3f. Stage 0 COLOROP is MODULATE or ADD; the alpha op stays MODULATE.
int __stdcall DeviceSetTextureStageState(void* self, unsigned stage, unsigned type,
    unsigned value)
{
    if (stage == 0 && type == kTssColorOp) {
        Device(self)->colorOp = value;
    }
    return kOk;
}

// Row vector times view times projection, then the viewport. D3D samples
// pixel centers at integer coordinates and SDL at +0.5.
struct Projector
{
    float m[16];
    Viewport vp;

    explicit Projector(const SdlDevice* device) : vp(device->viewport)
    {
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                float sum = 0;
                for (int k = 0; k < 4; ++k) {
                    sum += device->view[row * 4 + k] * device->proj[k * 4 + col];
                }
                m[row * 4 + col] = sum;
            }
        }
    }

    SDL_FPoint operator()(const Vertex& v) const
    {
        float x = v.x * m[0] + v.y * m[4] + v.z * m[8] + m[12];
        float y = v.x * m[1] + v.y * m[5] + v.z * m[9] + m[13];
        const float w = v.x * m[3] + v.y * m[7] + v.z * m[11] + m[15];
        if (w != 0.0f && w != 1.0f) {
            x /= w;
            y /= w;
        }
        return SDL_FPoint{vp.x + (x + 1.0f) * vp.width * 0.5f + 0.5f,
            vp.y + (1.0f - y) * vp.height * 0.5f + 0.5f};
    }
};

SDL_BlendMode BlendMode(const SdlDevice* device)
{
    return device->destBlend == kBlendOne ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND;
}

void BeginDraw(const SdlDevice* device)
{
    const Viewport& vp = device->viewport;
    const SDL_Rect clip = {int(vp.x), int(vp.y), int(vp.width), int(vp.height)};
    SDL_SetRenderClipRect(g_sdlRenderer, &clip);
}

const Vertex* Vertices(const SdlDevice* device, unsigned first, unsigned count)
{
    if (!device->stream ||
        (size_t(first) + count) * sizeof(Vertex) > device->stream->data.size()) {
        return 0;
    }
    return reinterpret_cast<const Vertex*>(device->stream->data.data()) + first;
}

// Diffuse at texture coordinate (u, v) of a quad, by barycentric weights.
SDL_FColor QuadColor(const Vertex* vertices, float u, float v)
{
    int corners[3] = {0, 1, 2};
    float weights[3] = {1, 0, 0};
    for (int triangle = 0; triangle < 2; ++triangle) {
        if (triangle) {
            corners[1] = 2;
            corners[2] = 3;
        }
        const Vertex& a = vertices[corners[0]];
        const Vertex& b = vertices[corners[1]];
        const Vertex& c = vertices[corners[2]];
        const float d = (b.v - c.v) * (a.u - c.u) + (c.u - b.u) * (a.v - c.v);
        if (std::fabs(d) < 0.000001f) {
            continue;
        }
        weights[0] = ((b.v - c.v) * (u - c.u) + (c.u - b.u) * (v - c.v)) / d;
        weights[1] = ((c.v - a.v) * (u - c.u) + (a.u - c.u) * (v - c.v)) / d;
        weights[2] = 1 - weights[0] - weights[1];
        if (weights[0] >= -0.001f && weights[1] >= -0.001f && weights[2] >= -0.001f) {
            break;
        }
    }
    SDL_FColor result = {0, 0, 0, 0};
    for (int i = 0; i < 3; ++i) {
        const SDL_FColor c = Color(vertices[corners[i]].color);
        result.r += c.r * weights[i];
        result.g += c.g * weights[i];
        result.b += c.b * weights[i];
        result.a += c.a * weights[i];
    }
    return result;
}

// D3DTOP_ADD saturates texture + diffuse before blending; SDL's geometry
// shader only multiplies, so the quad is shaded into a temporary texture.
SDL_Texture* AddColorTexture(SdlTexture* source, const Vertex* vertices)
{
    if (!ReadTarget(source)) {
        return 0;
    }
    std::vector<unsigned> pixels(source->pixels.size());
    for (unsigned y = 0; y < source->height; ++y) {
        for (unsigned x = 0; x < source->width; ++x) {
            const SDL_FColor tint = QuadColor(vertices, (x + 0.5f) / source->width,
                (y + 0.5f) / source->height);
            const unsigned p = source->pixels[y * source->width + x];
            auto byte = [](float value) {
                return unsigned((std::max)(0.f, (std::min)(255.f, value)) + 0.5f);
            };
            const unsigned r = byte(((p >> 16) & 255) + tint.r * 255);
            const unsigned g = byte(((p >> 8) & 255) + tint.g * 255);
            const unsigned b = byte((p & 255) + tint.b * 255);
            const unsigned a = byte((source->format == kX8R8G8B8 ? 255 : (p >> 24)) * tint.a);
            pixels[y * source->width + x] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }
    SDL_Texture* result = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STATIC, source->width, source->height);
    if (result) {
        SDL_UpdateTexture(result, 0, pixels.data(), source->width * 4);
        SDL_SetTextureScaleMode(result, SDL_SCALEMODE_LINEAR);
    }
    return result;
}

void DrawGeometry(SdlDevice* device, const Vertex* vertices, unsigned vertexCount,
    const int* indices, unsigned indexCount)
{
    const Projector project(device);
    std::vector<SDL_Vertex> out(vertexCount);
    for (unsigned i = 0; i < vertexCount; ++i) {
        const Vertex& v = vertices[i];
        out[i] = SDL_Vertex{project(v), Color(v.color), SDL_FPoint{v.u, v.v}};
    }
    SdlTexture* texture = device->texture;
    const SDL_BlendMode mode = BlendMode(device);
    BeginDraw(device);
    if (texture && texture->texture) {
        SDL_SetTextureBlendMode(texture->texture, mode);
    }
    else {
        SDL_SetRenderDrawBlendMode(g_sdlRenderer, mode);
    }
    if (!SDL_RenderGeometry(g_sdlRenderer, texture ? texture->texture : 0, out.data(),
            int(vertexCount), indices, int(indexCount))) {
        SDL_Log("SDL geometry: %s", SDL_GetError());
    }
}

// Quads of the index buffer (0, 1, 2, 2, 3, 0 per four vertices) with the
// texture-plus-diffuse color operation.
void DrawAddQuads(SdlDevice* device, const Vertex* vertices, const short* indices,
    unsigned quads)
{
    const Projector project(device);
    const int order[6] = {0, 1, 2, 2, 3, 0};
    const SDL_BlendMode mode = BlendMode(device);
    BeginDraw(device);
    for (unsigned q = 0; q < quads; ++q) {
        const Vertex* quad = vertices + indices[q * 6];
        SDL_Texture* shaded = AddColorTexture(device->texture, quad);
        if (!shaded) {
            SDL_Log("SDL texture color operation: %s", SDL_GetError());
            continue;
        }
        SDL_SetTextureBlendMode(shaded, mode);
        SDL_Vertex out[4];
        for (int i = 0; i < 4; ++i) {
            out[i] = SDL_Vertex{project(quad[i]), SDL_FColor{1, 1, 1, 1},
                SDL_FPoint{quad[i].u, quad[i].v}};
        }
        SDL_RenderGeometry(g_sdlRenderer, shaded, out, 4, order, 6);
        SDL_DestroyTexture(shaded);
    }
}

// Slot 0x46. Line lists (untextured) and triangle lists.
int __stdcall DeviceDrawPrimitive(void* self, int type, unsigned start, unsigned count)
{
    SdlDevice* device = Device(self);
    if (type == kLineList) {
        const Vertex* vertices = Vertices(device, start, count * 2);
        if (!vertices) {
            return kInvalidCall;
        }
        const Projector project(device);
        BeginDraw(device);
        SDL_SetRenderDrawBlendMode(g_sdlRenderer, BlendMode(device));
        for (unsigned i = 0; i < count; ++i) {
            const SDL_FPoint from = project(vertices[i * 2]);
            const SDL_FPoint to = project(vertices[i * 2 + 1]);
            const SDL_FColor c = Color(vertices[i * 2].color);
            SDL_SetRenderDrawColorFloat(g_sdlRenderer, c.r, c.g, c.b, c.a);
            SDL_RenderLine(g_sdlRenderer, from.x, from.y, to.x, to.y);
        }
        return kOk;
    }
    if (type == kTriangleList) {
        const Vertex* vertices = Vertices(device, start, count * 3);
        if (!vertices) {
            return kInvalidCall;
        }
        DrawGeometry(device, vertices, count * 3, 0, 0);
        return kOk;
    }
    return kInvalidCall;
}

// Slot 0x47. Indexed triangle lists from the bound index buffer.
int __stdcall DeviceDrawIndexedPrimitive(void* self, int type, unsigned minIndex,
    unsigned vertexCount, unsigned startIndex, unsigned primitives)
{
    SdlDevice* device = Device(self);
    const unsigned indexCount = primitives * 3;
    const Vertex* vertices = Vertices(device, 0, minIndex + vertexCount);
    if (type != kTriangleList || !vertices || !device->indices ||
        (size_t(startIndex) + indexCount) * 2 > device->indices->data.size()) {
        return kInvalidCall;
    }
    const short* indices = reinterpret_cast<const short*>(device->indices->data.data()) + startIndex;
    if (device->texture && device->colorOp == kTopAdd && startIndex % 6 == 0 &&
        primitives % 2 == 0) {
        DrawAddQuads(device, vertices, indices, primitives / 2);
        return kOk;
    }
    std::vector<int> wide(indexCount);
    for (unsigned i = 0; i < indexCount; ++i) {
        wide[i] = static_cast<unsigned short>(indices[i]);
        if (wide[i] >= int(minIndex + vertexCount)) {
            return kInvalidCall;
        }
    }
    DrawGeometry(device, vertices, minIndex + vertexCount, wide.data(), indexCount);
    return kOk;
}

// Slot 0x4c. FVF 0x142 only.
int __stdcall DeviceSetVertexShader(void*, unsigned fvf)
{
    return fvf == kFvf ? kOk : kInvalidCall;
}

// Slot 0x53.
int __stdcall DeviceSetStreamSource(void* self, unsigned stream, void* buffer, unsigned stride)
{
    if (stream || (buffer && stride != sizeof(Vertex))) {
        return kInvalidCall;
    }
    HoldBuffer(&Device(self)->stream, Buffer(buffer));
    return kOk;
}

// Slot 0x55.
int __stdcall DeviceSetIndices(void* self, void* buffer, unsigned base)
{
    if (base) {
        return kInvalidCall;
    }
    HoldBuffer(&Device(self)->indices, Buffer(buffer));
    return kOk;
}

// ---- IDirect3D8 ------------------------------------------------------------

unsigned long __stdcall D3dAddRef(void* self) { return ++static_cast<SdlD3d*>(self)->refs; }
unsigned long __stdcall D3dRelease(void* self)
{
    SdlD3d* d3d = static_cast<SdlD3d*>(self);
    if (--d3d->refs) {
        return d3d->refs;
    }
    delete d3d;
    return 0;
}

// Slot 5. D3DADAPTER_IDENTIFIER8: driver, description, driver version.
int __stdcall D3dGetAdapterIdentifier(void*, unsigned adapter, unsigned flags, unsigned* out)
{
    (void)adapter;
    (void)flags;
    memset(out, 0, 0x42c);
    SDL_strlcpy(reinterpret_cast<char*>(out), "SDL3", 0x200);
    const char* video = SDL_GetCurrentVideoDriver();
    SDL_strlcpy(reinterpret_cast<char*>(out) + 0x200, video ? video : "", 0x200);
    const int version = SDL_GetVersion();
    out[0x105] = (unsigned(SDL_VERSIONNUM_MAJOR(version)) << 16) | SDL_VERSIONNUM_MINOR(version);
    out[0x104] = unsigned(SDL_VERSIONNUM_MICRO(version)) << 16;
    return kOk;
}

// Slots 6/7/8. One mode: the desktop.
unsigned __stdcall D3dGetAdapterModeCount(void*, unsigned) { return 1; }

int __stdcall D3dEnumAdapterModes(void*, unsigned, unsigned index, DisplayMode* out)
{
    if (index) {
        return kInvalidCall;
    }
    DesktopMode(out);
    return kOk;
}

int __stdcall D3dGetAdapterDisplayMode(void*, unsigned, DisplayMode* out)
{
    DesktopMode(out);
    return kOk;
}

// Slot 0x0f. The focus window is the SDL window; vsync like the original's
// D3DPRESENT_INTERVAL_ONE.
int __stdcall D3dCreateDevice(void*, unsigned adapter, unsigned type, HWND window,
    unsigned behavior, const unsigned* present, void** out)
{
    (void)adapter;
    (void)type;
    (void)behavior;
    if (!out || g_sdlRenderer) {
        return kInvalidCall;
    }
    g_sdlRenderer = SDL_CreateRenderer(sdl3::WindowOf(window), 0);
    if (!g_sdlRenderer) {
        SDL_Log("SDL renderer: %s", SDL_GetError());
        return kInvalidCall;
    }
    SDL_SetRenderVSync(g_sdlRenderer, 1);
    SdlDevice* device = new SdlDevice();
    memset(device, 0, sizeof(*device));
    device->vtbl = s_deviceVtbl;
    device->refs = 1;
    Identity(device->view);
    Identity(device->proj);
    device->destBlend = 1;  // D3DBLEND_ZERO
    device->colorOp = kTopModulate;
    if (!CreateBackBuffer(device, present)) {
        DeviceRelease(device);
        return kInvalidCall;
    }
    *out = device;
    return kOk;
}

struct Tables
{
    Tables()
    {
        Fill<0>(s_d3dVtbl, std::make_index_sequence<0x10>());
        Set(s_d3dVtbl, 0x01, &D3dAddRef);
        Set(s_d3dVtbl, 0x02, &D3dRelease);
        Set(s_d3dVtbl, 0x05, &D3dGetAdapterIdentifier);
        Set(s_d3dVtbl, 0x06, &D3dGetAdapterModeCount);
        Set(s_d3dVtbl, 0x07, &D3dEnumAdapterModes);
        Set(s_d3dVtbl, 0x08, &D3dGetAdapterDisplayMode);
        Set(s_d3dVtbl, 0x0f, &D3dCreateDevice);

        Fill<1>(s_deviceVtbl, std::make_index_sequence<0x61>());
        Set(s_deviceVtbl, 0x01, &DeviceAddRef);
        Set(s_deviceVtbl, 0x02, &DeviceRelease);
        Set(s_deviceVtbl, 0x03, &DeviceTestCooperativeLevel);
        Set(s_deviceVtbl, 0x07, &DeviceGetDeviceCaps);
        Set(s_deviceVtbl, 0x0e, &DeviceReset);
        Set(s_deviceVtbl, 0x0f, &DevicePresent);
        Set(s_deviceVtbl, 0x14, &DeviceCreateTexture);
        Set(s_deviceVtbl, 0x17, &DeviceCreateVertexBuffer);
        Set(s_deviceVtbl, 0x18, &DeviceCreateIndexBuffer);
        Set(s_deviceVtbl, 0x1b, &DeviceCreateImageSurface);
        Set(s_deviceVtbl, 0x1c, &DeviceCopyRects);
        Set(s_deviceVtbl, 0x1f, &DeviceSetRenderTarget);
        Set(s_deviceVtbl, 0x20, &DeviceGetRenderTarget);
        Set(s_deviceVtbl, 0x22, &DeviceBeginScene);
        Set(s_deviceVtbl, 0x23, &DeviceEndScene);
        Set(s_deviceVtbl, 0x24, &DeviceClear);
        Set(s_deviceVtbl, 0x25, &DeviceSetTransform);
        Set(s_deviceVtbl, 0x28, &DeviceSetViewport);
        Set(s_deviceVtbl, 0x32, &DeviceSetRenderState);
        Set(s_deviceVtbl, 0x3d, &DeviceSetTexture);
        Set(s_deviceVtbl, 0x3f, &DeviceSetTextureStageState);
        Set(s_deviceVtbl, 0x46, &DeviceDrawPrimitive);
        Set(s_deviceVtbl, 0x47, &DeviceDrawIndexedPrimitive);
        Set(s_deviceVtbl, 0x4c, &DeviceSetVertexShader);
        Set(s_deviceVtbl, 0x53, &DeviceSetStreamSource);
        Set(s_deviceVtbl, 0x55, &DeviceSetIndices);

        Fill<2>(s_textureVtbl, std::make_index_sequence<0x13>());
        Set(s_textureVtbl, 0x01, &TextureAddRef);
        Set(s_textureVtbl, 0x02, &TextureRelease);
        Set(s_textureVtbl, 0x0e, &TextureGetLevelDesc);
        Set(s_textureVtbl, 0x0f, &TextureGetSurfaceLevel);
        Set(s_textureVtbl, 0x10, &TextureLockRect);
        Set(s_textureVtbl, 0x11, &TextureUnlockRect);

        Fill<3>(s_surfaceVtbl, std::make_index_sequence<0x0b>());
        Set(s_surfaceVtbl, 0x01, &SurfaceAddRef);
        Set(s_surfaceVtbl, 0x02, &SurfaceRelease);
        Set(s_surfaceVtbl, 0x09, &SurfaceLockRect);
        Set(s_surfaceVtbl, 0x0a, &SurfaceUnlockRect);

        Fill<4>(s_bufferVtbl, std::make_index_sequence<0x0e>());
        Set(s_bufferVtbl, 0x01, &BufferAddRef);
        Set(s_bufferVtbl, 0x02, &BufferRelease);
        Set(s_bufferVtbl, 0x0b, &BufferLock);
        Set(s_bufferVtbl, 0x0c, &BufferUnlock);
    }
} s_tables;

}

}

// The d3d8.dll export the recovered display code imports.
extern "C" vc1::IUnknown* __stdcall Direct3DCreate8(unsigned sdkVersion)
{
    (void)sdkVersion;
    vc1::SdlD3d* d3d = new vc1::SdlD3d();
    d3d->vtbl = vc1::s_d3dVtbl;
    d3d->refs = 1;
    return reinterpret_cast<vc1::IUnknown*>(d3d);
}
