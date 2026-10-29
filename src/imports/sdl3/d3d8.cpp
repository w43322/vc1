// D3D8 subset over SDL3 (see d3d8.h): IDirect3D8, IDirect3DDevice8,
// IDirect3DTexture8, IDirect3DSurface8 and the vertex and index buffers, as
// declared in kd/display/d3d8_api.h. The methods the recovered display code
// calls are implemented; the rest end the program with their name.
#include "d3d8.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace vc1 {

[[noreturn]] void D3dUnsupported(const char* method)
{
    SDL_Log("SDL D3D8: unsupported %s", method);
    abort();
}

namespace {

// The device's renderer; there is one device.
SDL_Renderer* g_sdlRenderer;

const D3dResult kOk = 0;
const D3dResult kInvalidCall = static_cast<D3dResult>(0x8876086cu);
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
const unsigned kLineList = 2;
const unsigned kTriangleList = 4;

// D3DPRESENT_PARAMETERS: the back buffer's width, height and format lead.
struct PresentParameters
{
    unsigned width;
    unsigned height;
    unsigned format;
};

// One FVF 0x142 vertex.
struct Vertex
{
    float x, y, z;
    unsigned color;
    float u, v;
};

SdlTexture* s_textures;  // every live texture, cleared when the renderer goes

SDL_FColor Color(unsigned c)
{
    return SDL_FColor{((c >> 16) & 255) / 255.f, ((c >> 8) & 255) / 255.f, (c & 255) / 255.f,
        (c >> 24) / 255.f};
}

// Adapter 0's desktop mode in pixels, reported as X8R8G8B8.
void DesktopMode(D3dDisplayMode* out)
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
    texture->texture = handle;
    texture->width = width;
    texture->height = height;
    texture->format = format;
    texture->target = target;
    texture->pixels.assign(size_t(width) * height, format == kX8R8G8B8 ? 0xff000000u : 0);
    if (!target) {
        SDL_UpdateTexture(handle, 0, texture->pixels.data(), width * 4);
    }
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

// Replaces a counted reference, as the device does for what it has bound.
template <class T>
void Hold(T** slot, T* object)
{
    if (object) {
        object->AddRef();
    }
    if (*slot) {
        (*slot)->Release();
    }
    *slot = object;
}

SdlSurface* NewSurface(SdlTexture* owner, unsigned width, unsigned height)
{
    SdlSurface* surface = new SdlSurface();
    surface->owner = owner;
    surface->width = width;
    surface->height = height;
    if (owner) {
        owner->AddRef();
    }
    else {
        surface->pixels.resize(size_t(width) * height);
    }
    return surface;
}

// ---- Vertex and index buffers ----------------------------------------------

// IDirect3DVertexBuffer8 / IDirect3DIndexBuffer8: plain memory. Draws read it
// directly, so Unlock has nothing to do.
template <class Interface>
class SdlBuffer final : public SdlResource<Interface>
{
public:
    unsigned refs = 1;
    std::vector<unsigned char> data;

    explicit SdlBuffer(unsigned bytes) : data(bytes) {}

    unsigned long __stdcall AddRef() override { return ++refs; }
    unsigned long __stdcall Release() override
    {
        if (--refs) {
            return refs;
        }
        delete this;
        return 0;
    }
    D3dResult __stdcall Lock(unsigned offset, unsigned size, unsigned char** out,
        unsigned long) override
    {
        if (!out || offset > data.size() || size > data.size() - offset) {
            return kInvalidCall;
        }
        *out = data.data() + offset;
        return kOk;
    }
    D3dResult __stdcall Unlock() override { return kOk; }
};

typedef SdlBuffer<IDirect3DVertexBuffer8> SdlVertexBuffer;
typedef SdlBuffer<IDirect3DIndexBuffer8> SdlIndexBuffer;

// ---- Device ----------------------------------------------------------------

void Identity(float* m)
{
    for (int i = 0; i < 16; ++i) {
        m[i] = (i % 5) == 0 ? 1.0f : 0.0f;
    }
}

#ifdef __EMSCRIPTEN__
// The browser shows one frame per animation frame. Waiting for the next one
// paces the game like the original's vsync (SDL only yields with a zero
// timeout) and leaves the page's thread time for input and the devtools.
EM_ASYNC_JS(void, WaitForAnimationFrame, (), {
    await new Promise(function (resolve) { requestAnimationFrame(function () { resolve(); }); });
});
#endif

// Row vector times view times projection, then the viewport. D3D samples
// pixel centers at integer coordinates and SDL at +0.5.
struct Projector
{
    float m[16];
    D3dViewport vp;

    Projector(const float* view, const float* proj, const D3dViewport& viewport)
        : vp(viewport)
    {
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                float sum = 0;
                for (int k = 0; k < 4; ++k) {
                    sum += view[row * 4 + k] * proj[k * 4 + col];
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

// Defines a device method the game never calls.
#define VC1_UNSUPPORTED(type, name, parameters) \
    type __stdcall name parameters override { D3dUnsupported("IDirect3DDevice8::" #name); }

// IDirect3DDevice8. Holds references to what is bound, as D3D does.
class SdlDevice final : public IDirect3DDevice8
{
public:
    unsigned refs = 1;
    SdlTexture* backBuffer = 0;
    SdlTexture* target = 0;
    SdlTexture* texture = 0;
    SdlVertexBuffer* stream = 0;
    SdlIndexBuffer* indices = 0;
    D3dViewport viewport = {0, 0, 0, 0, 0.0f, 1.0f};
    float view[16];
    float proj[16];
    unsigned long destBlend = 1;  // D3DBLEND_ZERO
    unsigned long colorOp = kTopModulate;

    SdlDevice()
    {
        Identity(view);
        Identity(proj);
    }

    D3dResult __stdcall QueryInterface(const void*, void**) override
    {
        D3dUnsupported("QueryInterface");
    }
    unsigned long __stdcall AddRef() override { return ++refs; }
    unsigned long __stdcall Release() override;

    // The SDL renderer is never lost.
    D3dResult __stdcall TestCooperativeLevel() override { return kOk; }
    VC1_UNSUPPORTED(unsigned, GetAvailableTextureMem, ())
    VC1_UNSUPPORTED(D3dResult, ResourceManagerDiscardBytes, (unsigned long))
    VC1_UNSUPPORTED(D3dResult, GetDirect3D, (void**))
    // No texture caps: sizes are not rounded to powers of two.
    D3dResult __stdcall GetDeviceCaps(unsigned char* caps) override
    {
        memset(caps, 0, 0xd4);
        return kOk;
    }
    VC1_UNSUPPORTED(D3dResult, GetDisplayMode, (D3dDisplayMode*))
    VC1_UNSUPPORTED(D3dResult, GetCreationParameters, (void*))
    VC1_UNSUPPORTED(D3dResult, SetCursorProperties, (unsigned, unsigned, IDirect3DSurface8*))
    VC1_UNSUPPORTED(void, SetCursorPosition, (int, int, unsigned long))
    VC1_UNSUPPORTED(int, ShowCursor, (int))
    VC1_UNSUPPORTED(D3dResult, CreateAdditionalSwapChain, (void*, void**))
    // A new back buffer at the presentation size.
    D3dResult __stdcall Reset(void* parameters) override
    {
        return CreateBackBuffer(static_cast<const PresentParameters*>(parameters)) ? kOk
                                                                                  : kInvalidCall;
    }
    D3dResult __stdcall Present(const tagRECT*, const tagRECT*, HWND__*, const void*) override;
    VC1_UNSUPPORTED(D3dResult, GetBackBuffer, (unsigned, unsigned, IDirect3DSurface8**))
    VC1_UNSUPPORTED(D3dResult, GetRasterStatus, (void*))
    VC1_UNSUPPORTED(void, SetGammaRamp, (unsigned long, const void*))
    VC1_UNSUPPORTED(void, GetGammaRamp, (void*))
    D3dResult __stdcall CreateTexture(unsigned width, unsigned height, unsigned levels,
        unsigned long usage, unsigned format, unsigned pool, IDirect3DTexture8** out) override
    {
        (void)levels;
        (void)pool;
        SdlTexture* created = NewTexture(width, height, format, (usage & kUsageRenderTarget) != 0);
        if (!out || !created) {
            return kInvalidCall;
        }
        *out = created;
        return kOk;
    }
    VC1_UNSUPPORTED(D3dResult, CreateVolumeTexture, (unsigned, unsigned, unsigned, unsigned,
        unsigned long, unsigned, unsigned, void**))
    VC1_UNSUPPORTED(D3dResult, CreateCubeTexture, (unsigned, unsigned, unsigned long, unsigned,
        unsigned, void**))
    D3dResult __stdcall CreateVertexBuffer(unsigned length, unsigned long, unsigned long fvf,
        unsigned, IDirect3DVertexBuffer8** out) override
    {
        if (!out || fvf != kFvf) {
            return kInvalidCall;
        }
        *out = new SdlVertexBuffer(length);
        return kOk;
    }
    D3dResult __stdcall CreateIndexBuffer(unsigned length, unsigned long, unsigned format,
        unsigned, IDirect3DIndexBuffer8** out) override
    {
        if (!out || format != kIndex16) {
            return kInvalidCall;
        }
        *out = new SdlIndexBuffer(length);
        return kOk;
    }
    VC1_UNSUPPORTED(D3dResult, CreateRenderTarget, (unsigned, unsigned, unsigned, unsigned, int,
        IDirect3DSurface8**))
    VC1_UNSUPPORTED(D3dResult, CreateDepthStencilSurface, (unsigned, unsigned, unsigned, unsigned,
        IDirect3DSurface8**))
    D3dResult __stdcall CreateImageSurface(unsigned width, unsigned height, unsigned format,
        IDirect3DSurface8** out) override
    {
        if (!out || !width || !height || (format != kA8R8G8B8 && format != kX8R8G8B8)) {
            return kInvalidCall;
        }
        *out = NewSurface(0, width, height);
        return kOk;
    }
    D3dResult __stdcall CopyRects(IDirect3DSurface8* source, const tagRECT* rects, unsigned count,
        IDirect3DSurface8* destination, const tagPOINT* points) override;
    VC1_UNSUPPORTED(D3dResult, UpdateTexture, (IDirect3DBaseTexture8*, IDirect3DBaseTexture8*))
    VC1_UNSUPPORTED(D3dResult, GetFrontBuffer, (IDirect3DSurface8*))
    D3dResult __stdcall SetRenderTarget(IDirect3DSurface8* surface,
        IDirect3DSurface8* depth) override;
    D3dResult __stdcall GetRenderTarget(IDirect3DSurface8** out) override
    {
        if (!out || !target) {
            return kInvalidCall;
        }
        *out = NewSurface(target, target->width, target->height);
        return kOk;
    }
    VC1_UNSUPPORTED(D3dResult, GetDepthStencilSurface, (IDirect3DSurface8**))
    // Draws are immediate.
    D3dResult __stdcall BeginScene() override { return kOk; }
    D3dResult __stdcall EndScene() override { return kOk; }
    D3dResult __stdcall Clear(unsigned long count, const void* rects, unsigned long flags,
        unsigned long color, float z, unsigned long stencil) override;
    // Only the view and projection transforms are set (world stays identity).
    D3dResult __stdcall SetTransform(unsigned state, const float* matrix) override
    {
        if (state == kTsView) {
            memcpy(view, matrix, sizeof(view));
        }
        else if (state == kTsProjection) {
            memcpy(proj, matrix, sizeof(proj));
        }
        return kOk;
    }
    VC1_UNSUPPORTED(D3dResult, GetTransform, (unsigned, float*))
    VC1_UNSUPPORTED(D3dResult, MultiplyTransform, (unsigned, const float*))
    D3dResult __stdcall SetViewport(const D3dViewport* value) override
    {
        viewport = *value;
        return kOk;
    }
    VC1_UNSUPPORTED(D3dResult, GetViewport, (D3dViewport*))
    VC1_UNSUPPORTED(D3dResult, SetMaterial, (const void*))
    VC1_UNSUPPORTED(D3dResult, GetMaterial, (void*))
    VC1_UNSUPPORTED(D3dResult, SetLight, (unsigned long, const void*))
    VC1_UNSUPPORTED(D3dResult, GetLight, (unsigned long, void*))
    VC1_UNSUPPORTED(D3dResult, LightEnable, (unsigned long, int))
    VC1_UNSUPPORTED(D3dResult, GetLightEnable, (unsigned long, int*))
    VC1_UNSUPPORTED(D3dResult, SetClipPlane, (unsigned long, const float*))
    VC1_UNSUPPORTED(D3dResult, GetClipPlane, (unsigned long, float*))
    // The source blend stays SRCALPHA and Z is off, so only the destination
    // blend (INVSRCALPHA or ONE) changes what is drawn.
    D3dResult __stdcall SetRenderState(unsigned state, unsigned long value) override
    {
        if (state == kRsDestBlend) {
            destBlend = value;
        }
        return kOk;
    }
    VC1_UNSUPPORTED(D3dResult, GetRenderState, (unsigned, unsigned long*))
    VC1_UNSUPPORTED(D3dResult, BeginStateBlock, ())
    VC1_UNSUPPORTED(D3dResult, EndStateBlock, (unsigned long*))
    VC1_UNSUPPORTED(D3dResult, ApplyStateBlock, (unsigned long))
    VC1_UNSUPPORTED(D3dResult, CaptureStateBlock, (unsigned long))
    VC1_UNSUPPORTED(D3dResult, DeleteStateBlock, (unsigned long))
    VC1_UNSUPPORTED(D3dResult, CreateStateBlock, (unsigned, unsigned long*))
    VC1_UNSUPPORTED(D3dResult, SetClipStatus, (const void*))
    VC1_UNSUPPORTED(D3dResult, GetClipStatus, (void*))
    VC1_UNSUPPORTED(D3dResult, GetTexture, (unsigned long, IDirect3DBaseTexture8**))
    // Stage 0 only; every texture is an SdlTexture.
    D3dResult __stdcall SetTexture(unsigned long stage, IDirect3DBaseTexture8* value) override
    {
        if (stage) {
            return kInvalidCall;
        }
        Hold(&texture, static_cast<SdlTexture*>(value));
        return kOk;
    }
    VC1_UNSUPPORTED(D3dResult, GetTextureStageState, (unsigned long, unsigned, unsigned long*))
    // Stage 0 COLOROP is MODULATE or ADD; the alpha op stays MODULATE.
    D3dResult __stdcall SetTextureStageState(unsigned long stage, unsigned type,
        unsigned long value) override
    {
        if (stage == 0 && type == kTssColorOp) {
            colorOp = value;
        }
        return kOk;
    }
    VC1_UNSUPPORTED(D3dResult, ValidateDevice, (unsigned long*))
    VC1_UNSUPPORTED(D3dResult, GetInfo, (unsigned long, void*, unsigned long))
    VC1_UNSUPPORTED(D3dResult, SetPaletteEntries, (unsigned, const void*))
    VC1_UNSUPPORTED(D3dResult, GetPaletteEntries, (unsigned, void*))
    VC1_UNSUPPORTED(D3dResult, SetCurrentTexturePalette, (unsigned))
    VC1_UNSUPPORTED(D3dResult, GetCurrentTexturePalette, (unsigned*))
    D3dResult __stdcall DrawPrimitive(unsigned type, unsigned start, unsigned count) override;
    D3dResult __stdcall DrawIndexedPrimitive(unsigned type, unsigned minIndex,
        unsigned vertexCount, unsigned startIndex, unsigned primitives) override;
    VC1_UNSUPPORTED(D3dResult, DrawPrimitiveUP, (unsigned, unsigned, const void*, unsigned))
    VC1_UNSUPPORTED(D3dResult, DrawIndexedPrimitiveUP, (unsigned, unsigned, unsigned, unsigned,
        const void*, unsigned, const void*, unsigned))
    VC1_UNSUPPORTED(D3dResult, ProcessVertices, (unsigned, unsigned, unsigned,
        IDirect3DVertexBuffer8*, unsigned long))
    VC1_UNSUPPORTED(D3dResult, CreateVertexShader, (const unsigned long*, const unsigned long*,
        unsigned long*, unsigned long))
    // FVF 0x142 only.
    D3dResult __stdcall SetVertexShader(unsigned long fvf) override
    {
        return fvf == kFvf ? kOk : kInvalidCall;
    }
    VC1_UNSUPPORTED(D3dResult, GetVertexShader, (unsigned long*))
    VC1_UNSUPPORTED(D3dResult, DeleteVertexShader, (unsigned long))
    VC1_UNSUPPORTED(D3dResult, SetVertexShaderConstant, (unsigned long, const void*,
        unsigned long))
    VC1_UNSUPPORTED(D3dResult, GetVertexShaderConstant, (unsigned long, void*, unsigned long))
    VC1_UNSUPPORTED(D3dResult, GetVertexShaderDeclaration, (unsigned long, void*,
        unsigned long*))
    VC1_UNSUPPORTED(D3dResult, GetVertexShaderFunction, (unsigned long, void*, unsigned long*))
    D3dResult __stdcall SetStreamSource(unsigned index, IDirect3DVertexBuffer8* buffer,
        unsigned stride) override
    {
        if (index || (buffer && stride != sizeof(Vertex))) {
            return kInvalidCall;
        }
        Hold(&stream, static_cast<SdlVertexBuffer*>(buffer));
        return kOk;
    }
    VC1_UNSUPPORTED(D3dResult, GetStreamSource, (unsigned, IDirect3DVertexBuffer8**, unsigned*))
    D3dResult __stdcall SetIndices(IDirect3DIndexBuffer8* buffer, unsigned base) override
    {
        if (base) {
            return kInvalidCall;
        }
        Hold(&indices, static_cast<SdlIndexBuffer*>(buffer));
        return kOk;
    }

    bool CreateBackBuffer(const PresentParameters* present);

private:
    void FullViewport()
    {
        viewport = D3dViewport{0, 0, target ? target->width : 0, target ? target->height : 0,
            0.0f, 1.0f};
    }
    Projector Project() const { return Projector(view, proj, viewport); }
    SDL_BlendMode BlendMode() const
    {
        return destBlend == kBlendOne ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND;
    }
    void BeginDraw() const
    {
        const SDL_Rect clip = {int(viewport.x), int(viewport.y), int(viewport.width),
            int(viewport.height)};
        SDL_SetRenderClipRect(g_sdlRenderer, &clip);
    }
    const Vertex* Vertices(unsigned first, unsigned count) const
    {
        if (!stream || (size_t(first) + count) * sizeof(Vertex) > stream->data.size()) {
            return 0;
        }
        return reinterpret_cast<const Vertex*>(stream->data.data()) + first;
    }
    void DrawGeometry(const Vertex* vertices, unsigned vertexCount, const int* order,
        unsigned indexCount);
    void DrawAddQuads(const Vertex* vertices, const short* order, unsigned quads);
};

#undef VC1_UNSUPPORTED

unsigned long __stdcall SdlDevice::Release()
{
    if (--refs) {
        return refs;
    }
    Hold<SdlTexture>(&texture, 0);
    Hold<SdlTexture>(&target, 0);
    Hold<SdlTexture>(&backBuffer, 0);
    Hold<SdlVertexBuffer>(&stream, 0);
    Hold<SdlIndexBuffer>(&indices, 0);
    // Textures the game still holds outlive the renderer without handles.
    for (SdlTexture* live = s_textures; live; live = live->next) {
        if (live->texture) {
            SDL_DestroyTexture(live->texture);
        }
        live->texture = 0;
    }
    SDL_DestroyRenderer(g_sdlRenderer);
    g_sdlRenderer = 0;
    delete this;
    return 0;
}

bool SdlDevice::CreateBackBuffer(const PresentParameters* present)
{
    SdlTexture* created = NewTexture(present->width, present->height, present->format, true);
    if (!created) {
        return false;
    }
    Hold(&backBuffer, created);
    created->Release();
    Hold(&target, created);
    SDL_SetRenderTarget(g_sdlRenderer, created->texture);
    FullViewport();
    return true;
}

// Scales the finished back buffer into the window; the game rasterizes at its
// own resolution, so tiles never meet at fractional pixels.
D3dResult __stdcall SdlDevice::Present(const tagRECT*, const tagRECT*, HWND__*, const void*)
{
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
    SDL_SetRenderTarget(g_sdlRenderer, target ? target->texture : 0);
    return kOk;
}

// Copies from a texture level into an image surface.
D3dResult __stdcall SdlDevice::CopyRects(IDirect3DSurface8* source, const tagRECT* rects,
    unsigned count, IDirect3DSurface8* destination, const tagPOINT* points)
{
    SdlSurface* from = static_cast<SdlSurface*>(source);
    SdlSurface* to = static_cast<SdlSurface*>(destination);
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

// The viewport resets to the whole target.
D3dResult __stdcall SdlDevice::SetRenderTarget(IDirect3DSurface8* surface,
    IDirect3DSurface8* depth)
{
    (void)depth;
    SdlTexture* owner = surface ? static_cast<SdlSurface*>(surface)->owner : 0;
    if (!owner || !owner->target || !owner->texture ||
        !SDL_SetRenderTarget(g_sdlRenderer, owner->texture)) {
        return kInvalidCall;
    }
    Hold(&target, owner);
    FullViewport();
    return kOk;
}

// Fills the viewport.
D3dResult __stdcall SdlDevice::Clear(unsigned long count, const void* rects,
    unsigned long flags, unsigned long color, float z, unsigned long stencil)
{
    (void)z;
    (void)stencil;
    if (count || rects) {
        return kInvalidCall;
    }
    if (flags & kClearTarget) {
        const SDL_FRect area = {float(viewport.x), float(viewport.y), float(viewport.width),
            float(viewport.height)};
        const SDL_FColor c = Color(color);
        SDL_SetRenderClipRect(g_sdlRenderer, 0);
        SDL_SetRenderDrawBlendMode(g_sdlRenderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColorFloat(g_sdlRenderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(g_sdlRenderer, &area);
    }
    return kOk;
}

void SdlDevice::DrawGeometry(const Vertex* vertices, unsigned vertexCount, const int* order,
    unsigned indexCount)
{
    const Projector project = Project();
    std::vector<SDL_Vertex> out(vertexCount);
    for (unsigned i = 0; i < vertexCount; ++i) {
        const Vertex& v = vertices[i];
        out[i] = SDL_Vertex{project(v), Color(v.color), SDL_FPoint{v.u, v.v}};
    }
    const SDL_BlendMode mode = BlendMode();
    BeginDraw();
    if (texture && texture->texture) {
        SDL_SetTextureBlendMode(texture->texture, mode);
    }
    else {
        SDL_SetRenderDrawBlendMode(g_sdlRenderer, mode);
    }
    if (!SDL_RenderGeometry(g_sdlRenderer, texture ? texture->texture : 0, out.data(),
            int(vertexCount), order, int(indexCount))) {
        SDL_Log("SDL geometry: %s", SDL_GetError());
    }
}

// Quads of the index buffer (0, 1, 2, 2, 3, 0 per four vertices) with the
// texture-plus-diffuse color operation.
void SdlDevice::DrawAddQuads(const Vertex* vertices, const short* order, unsigned quads)
{
    const Projector project = Project();
    const int corners[6] = {0, 1, 2, 2, 3, 0};
    const SDL_BlendMode mode = BlendMode();
    BeginDraw();
    for (unsigned q = 0; q < quads; ++q) {
        const Vertex* quad = vertices + order[q * 6];
        SDL_Texture* shaded = AddColorTexture(texture, quad);
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
        SDL_RenderGeometry(g_sdlRenderer, shaded, out, 4, corners, 6);
        SDL_DestroyTexture(shaded);
    }
}

// Line lists (untextured) and triangle lists.
D3dResult __stdcall SdlDevice::DrawPrimitive(unsigned type, unsigned start, unsigned count)
{
    if (type == kLineList) {
        const Vertex* vertices = Vertices(start, count * 2);
        if (!vertices) {
            return kInvalidCall;
        }
        const Projector project = Project();
        BeginDraw();
        SDL_SetRenderDrawBlendMode(g_sdlRenderer, BlendMode());
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
        const Vertex* vertices = Vertices(start, count * 3);
        if (!vertices) {
            return kInvalidCall;
        }
        DrawGeometry(vertices, count * 3, 0, 0);
        return kOk;
    }
    return kInvalidCall;
}

// Indexed triangle lists from the bound index buffer.
D3dResult __stdcall SdlDevice::DrawIndexedPrimitive(unsigned type, unsigned minIndex,
    unsigned vertexCount, unsigned startIndex, unsigned primitives)
{
    const unsigned indexCount = primitives * 3;
    const Vertex* vertices = Vertices(0, minIndex + vertexCount);
    if (type != kTriangleList || !vertices || !indices ||
        (size_t(startIndex) + indexCount) * 2 > indices->data.size()) {
        return kInvalidCall;
    }
    const short* order = reinterpret_cast<const short*>(indices->data.data()) + startIndex;
    if (texture && colorOp == kTopAdd && startIndex % 6 == 0 && primitives % 2 == 0) {
        DrawAddQuads(vertices, order, primitives / 2);
        return kOk;
    }
    std::vector<int> wide(indexCount);
    for (unsigned i = 0; i < indexCount; ++i) {
        wide[i] = static_cast<unsigned short>(order[i]);
        if (wide[i] >= int(minIndex + vertexCount)) {
            return kInvalidCall;
        }
    }
    DrawGeometry(vertices, minIndex + vertexCount, wide.data(), indexCount);
    return kOk;
}

// ---- IDirect3D8 ------------------------------------------------------------

#define VC1_UNSUPPORTED(type, name, parameters) \
    type __stdcall name parameters override { D3dUnsupported("IDirect3D8::" #name); }

class SdlD3d final : public IDirect3D8
{
public:
    unsigned refs = 1;

    D3dResult __stdcall QueryInterface(const void*, void**) override
    {
        D3dUnsupported("QueryInterface");
    }
    unsigned long __stdcall AddRef() override { return ++refs; }
    unsigned long __stdcall Release() override
    {
        if (--refs) {
            return refs;
        }
        delete this;
        return 0;
    }
    VC1_UNSUPPORTED(D3dResult, RegisterSoftwareDevice, (void*))
    VC1_UNSUPPORTED(unsigned, GetAdapterCount, ())
    // D3DADAPTER_IDENTIFIER8: driver, description, driver version.
    D3dResult __stdcall GetAdapterIdentifier(unsigned, unsigned long, void* identifier) override
    {
        char* const text = static_cast<char*>(identifier);
        unsigned* const words = static_cast<unsigned*>(identifier);
        memset(identifier, 0, 0x42c);
        SDL_strlcpy(text, "SDL3", 0x200);
        const char* video = SDL_GetCurrentVideoDriver();
        SDL_strlcpy(text + 0x200, video ? video : "", 0x200);
        const int version = SDL_GetVersion();
        words[0x105] = (unsigned(SDL_VERSIONNUM_MAJOR(version)) << 16) |
            SDL_VERSIONNUM_MINOR(version);
        words[0x104] = unsigned(SDL_VERSIONNUM_MICRO(version)) << 16;
        return kOk;
    }
    // One mode: the desktop.
    unsigned __stdcall GetAdapterModeCount(unsigned) override { return 1; }
    D3dResult __stdcall EnumAdapterModes(unsigned, unsigned mode, D3dDisplayMode* out) override
    {
        if (mode) {
            return kInvalidCall;
        }
        DesktopMode(out);
        return kOk;
    }
    D3dResult __stdcall GetAdapterDisplayMode(unsigned, D3dDisplayMode* out) override
    {
        DesktopMode(out);
        return kOk;
    }
    VC1_UNSUPPORTED(D3dResult, CheckDeviceType, (unsigned, unsigned, unsigned, unsigned, int))
    VC1_UNSUPPORTED(D3dResult, CheckDeviceFormat, (unsigned, unsigned, unsigned, unsigned long,
        unsigned, unsigned))
    VC1_UNSUPPORTED(D3dResult, CheckDeviceMultiSampleType, (unsigned, unsigned, unsigned, int,
        unsigned))
    VC1_UNSUPPORTED(D3dResult, CheckDepthStencilMatch, (unsigned, unsigned, unsigned, unsigned,
        unsigned))
    VC1_UNSUPPORTED(D3dResult, GetDeviceCaps, (unsigned, unsigned, unsigned char*))
    VC1_UNSUPPORTED(void*, GetAdapterMonitor, (unsigned))
    // The focus window is the SDL window; vsync like the original's
    // D3DPRESENT_INTERVAL_ONE.
    D3dResult __stdcall CreateDevice(unsigned, unsigned, HWND__* window, unsigned long,
        void* parameters, IDirect3DDevice8** out) override
    {
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
        if (!device->CreateBackBuffer(static_cast<const PresentParameters*>(parameters))) {
            device->Release();
            return kInvalidCall;
        }
        *out = device;
        return kOk;
    }
};

#undef VC1_UNSUPPORTED

}

// ---- Textures and surfaces -------------------------------------------------

unsigned long __stdcall SdlTexture::AddRef()
{
    return ++refs;
}

unsigned long __stdcall SdlTexture::Release()
{
    if (--refs) {
        return refs;
    }
    if (prev) {
        prev->next = next;
    }
    else {
        s_textures = next;
    }
    if (next) {
        next->prev = prev;
    }
    if (texture) {
        SDL_DestroyTexture(texture);
    }
    delete this;
    return 0;
}

D3dResult __stdcall SdlTexture::GetLevelDesc(unsigned level, D3dSurfaceDesc* desc)
{
    if (level || !desc) {
        return kInvalidCall;
    }
    desc->format = format;
    desc->type = 1;  // D3DRTYPE_SURFACE
    desc->usage = target ? kUsageRenderTarget : 0;
    desc->pool = target ? 0 : 1;  // D3DPOOL_DEFAULT / MANAGED
    desc->size = width * height * 4;
    desc->multiSample = 0;
    desc->width = width;
    desc->height = height;
    return kOk;
}

// The surface holds a reference to its texture.
D3dResult __stdcall SdlTexture::GetSurfaceLevel(unsigned level, IDirect3DSurface8** surface)
{
    if (level || !surface) {
        return kInvalidCall;
    }
    *surface = NewSurface(this, width, height);
    return kOk;
}

// Managed textures only, one lock at a time; the bits are packed rows of the
// locked region.
D3dResult __stdcall SdlTexture::LockRect(unsigned level, D3dLockedRect* locked,
    const tagRECT* rect, unsigned long flags)
{
    if (level || !locked || target || !lock.empty()) {
        return kInvalidCall;
    }
    SDL_Rect r = {0, 0, int(width), int(height)};
    if (rect) {
        r = SDL_Rect{int(rect->left), int(rect->top), int(rect->right - rect->left),
            int(rect->bottom - rect->top)};
    }
    if (r.x < 0 || r.y < 0 || r.w <= 0 || r.h <= 0 || r.x + r.w > int(width) ||
        r.y + r.h > int(height)) {
        return kInvalidCall;
    }
    region = r;
    writable = (flags & kLockReadOnly) == 0;
    lock.resize(size_t(r.w) * r.h);
    for (int row = 0; row < r.h; ++row) {
        memcpy(&lock[size_t(row) * r.w], &pixels[size_t(r.y + row) * width + r.x],
            size_t(r.w) * 4);
    }
    locked->pitch = r.w * 4;
    locked->bits = lock.data();
    return kOk;
}

D3dResult __stdcall SdlTexture::UnlockRect(unsigned level)
{
    if (level || lock.empty()) {
        return kInvalidCall;
    }
    const SDL_Rect& r = region;
    if (writable) {
        for (int row = 0; row < r.h; ++row) {
            memcpy(&pixels[size_t(r.y + row) * width + r.x], &lock[size_t(row) * r.w],
                size_t(r.w) * 4);
        }
        SDL_UpdateTexture(texture, &r, lock.data(), r.w * 4);
    }
    lock.clear();
    return kOk;
}

unsigned long __stdcall SdlSurface::AddRef()
{
    return ++refs;
}

unsigned long __stdcall SdlSurface::Release()
{
    if (--refs) {
        return refs;
    }
    if (owner) {
        owner->Release();
    }
    delete this;
    return 0;
}

// Only image surfaces have their own memory.
D3dResult __stdcall SdlSurface::LockRect(D3dLockedRect* locked, const tagRECT* rect,
    unsigned long flags)
{
    (void)flags;
    if (!locked || owner) {
        return kInvalidCall;
    }
    locked->pitch = width * 4;
    locked->bits = pixels.data() + (rect ? rect->top * width + rect->left : 0);
    return kOk;
}

D3dResult __stdcall SdlSurface::UnlockRect()
{
    return kOk;
}

}

// The d3d8.dll export the recovered display code imports.
extern "C" vc1::IDirect3D8* __stdcall Direct3DCreate8(unsigned sdkVersion)
{
    (void)sdkVersion;
    return new vc1::SdlD3d();
}
