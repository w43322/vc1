#include "platform/sdl3/render.h"
#include "host/platform.h"
#include "host/window.h"
#include "app/script.h"
#include <cstdlib>
#include <cstring>
#include <set>

namespace vc1 {
void SdlUpdateAudio();
void* g_kdWindowMixer;
extern const char kKdParentWindow[] = "KD_PARENT_WINDOW";
struct SdlWindowState { void* userptr; SDL_Window* window; int cursor; };
static unsigned pointerButtons;
static std::set<unsigned> pressedKeys;
static SDL_Cursor* cursors[6];
static SDL_Surface* Resource(const char* name,unsigned id,int* hx,int* hy)
{
    char path[1024];SDL_snprintf(path,sizeof(path),"%splatform-resources/%s%u.bin",SDL_GetBasePath(),name,id);
    size_t bytes=0;unsigned* data=static_cast<unsigned*>(SDL_LoadFile(path,&bytes));
    if(!data) return nullptr;
    SDL_Surface* surface=nullptr;
    if(bytes>=20 && !memcmp(data,"VCR1",4) && data[1] && data[2]
        && bytes==20+size_t(data[1])*data[2]*4) {
        surface=SDL_CreateSurface(data[1],data[2],SDL_PIXELFORMAT_ARGB8888);
        if(surface) {
            for(unsigned y=0;y<data[2];++y) memcpy(static_cast<char*>(surface->pixels)+y*surface->pitch,data+5+y*data[1],data[1]*4);
            if(hx) *hx=data[3];if(hy) *hy=data[4];
        }
    }
    SDL_free(data);return surface;
}

void* CreateKdWindow()
{
    SdlWindowState* state=static_cast<SdlWindowState*>(calloc(1,sizeof(SdlWindowState)));
    state->userptr=state;
    state->window=SDL_CreateWindow("Virtual City",1024,768,SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if(!state->window) { free(state);return 0; }
    g_sdlWindow=state->window;g_kdWindowMixer=state;
    SDL_Surface* icon=Resource("icon",100,nullptr,nullptr);
    if(icon) {SDL_SetWindowIcon(g_sdlWindow,icon);SDL_DestroySurface(icon);}
    SDL_StartTextInput(g_sdlWindow);
    return state;
}
bool SdlCreateDisplay(unsigned width,unsigned height)
{
    if(!g_sdlWindow && !CreateKdWindow()) return false;
    SDL_SetWindowSize(g_sdlWindow,width,height);
    g_sdlRenderer=SDL_CreateRenderer(g_sdlWindow,0);
    if(!g_sdlRenderer) return false;
    SDL_SetRenderVSync(g_sdlRenderer,1);
    SDL_SetRenderLogicalPresentation(g_sdlRenderer,width,height,SDL_LOGICAL_PRESENTATION_LETTERBOX);
    return true;
}
void SdlDestroyDisplay()
{
    for(unsigned i=0;i<6;++i) { if(cursors[i]) SDL_DestroyCursor(cursors[i]);cursors[i]=0; }
    SDL_DestroyRenderer(g_sdlRenderer);g_sdlRenderer=0;
    SDL_DestroyWindow(g_sdlWindow);g_sdlWindow=0;
    free(g_kdWindowMixer);g_kdWindowMixer=0;
    pressedKeys.clear();pointerButtons=0;
}
void ScriptSetCursor(int cursor)
{
    if(!g_kdWindowMixer) return;
    static_cast<SdlWindowState*>(g_kdWindowMixer)->cursor=cursor;
    if(cursor<100 || cursor>105) return;
    unsigned index=cursor-100;
    if(!cursors[index]) {
        int hx=0,hy=0;SDL_Surface* resource=Resource("cursor",cursor,&hx,&hy);
        if(resource) {cursors[index]=SDL_CreateColorCursor(resource,hx,hy);SDL_DestroySurface(resource);}
    }
    if(!cursors[index]) {
        const SDL_SystemCursor kinds[6]={SDL_SYSTEM_CURSOR_DEFAULT,SDL_SYSTEM_CURSOR_POINTER,
            SDL_SYSTEM_CURSOR_CROSSHAIR,SDL_SYSTEM_CURSOR_MOVE,SDL_SYSTEM_CURSOR_NOT_ALLOWED,SDL_SYSTEM_CURSOR_WAIT};
        cursors[index]=SDL_CreateSystemCursor(kinds[index]);
    }
    if(cursors[index]) SDL_SetCursor(cursors[index]);
}
static void Post(int type,unsigned a,unsigned b,unsigned c,unsigned d)
{
    KdEvent* event=AllocEvent();if(!event) return;
    event->type=type;event->userptr=g_kdWindowMixer;
    event->data[0]=a;event->data[1]=b;event->data[2]=c;event->data[3]=d;
    if(type==0x39 && a==0x4001) DropPointerMoves(event);
    QueueEvent(event);
}
static unsigned VirtualKey(SDL_Keycode key)
{
    if(key>=SDLK_A && key<=SDLK_Z) return key-32;
    if(key>=SDLK_0 && key<=SDLK_9) return key;
    if(key>=SDLK_F1 && key<=SDLK_F12) return 0x70+key-SDLK_F1;
    if(key>=SDLK_F13 && key<=SDLK_F24) return 0x7c+key-SDLK_F13;
    switch(key) {
    case SDLK_RETURN:return 13;case SDLK_ESCAPE:return 27;case SDLK_SPACE:return 32;
    case SDLK_BACKSPACE:return 8;case SDLK_TAB:return 9;
    case SDLK_LEFT:return 37;case SDLK_UP:return 38;case SDLK_RIGHT:return 39;case SDLK_DOWN:return 40;
    case SDLK_HOME:return 36;case SDLK_END:return 35;case SDLK_PAGEUP:return 33;case SDLK_PAGEDOWN:return 34;
    case SDLK_INSERT:return 45;case SDLK_DELETE:return 46;
    case SDLK_LSHIFT:case SDLK_RSHIFT:return 16;
    case SDLK_LCTRL:case SDLK_RCTRL:return 17;case SDLK_LALT:case SDLK_RALT:return 18;
    case SDLK_CAPSLOCK:return 20;case SDLK_PAUSE:return 19;
    case SDLK_SEMICOLON:return 0xba;case SDLK_EQUALS:return 0xbb;
    case SDLK_COMMA:return 0xbc;case SDLK_MINUS:return 0xbd;case SDLK_PERIOD:return 0xbe;
    case SDLK_SLASH:return 0xbf;case SDLK_GRAVE:return 0xc0;
    case SDLK_LEFTBRACKET:return 0xdb;case SDLK_BACKSLASH:return 0xdc;
    case SDLK_RIGHTBRACKET:return 0xdd;case SDLK_APOSTROPHE:return 0xde;
    case SDLK_KP_0:return 0x60;case SDLK_KP_1:return 0x61;case SDLK_KP_2:return 0x62;
    case SDLK_KP_3:return 0x63;case SDLK_KP_4:return 0x64;case SDLK_KP_5:return 0x65;
    case SDLK_KP_6:return 0x66;case SDLK_KP_7:return 0x67;case SDLK_KP_8:return 0x68;case SDLK_KP_9:return 0x69;
    case SDLK_KP_MULTIPLY:return 0x6a;case SDLK_KP_PLUS:return 0x6b;case SDLK_KP_MINUS:return 0x6d;
    case SDLK_KP_PERIOD:return 0x6e;case SDLK_KP_DIVIDE:return 0x6f;case SDLK_KP_ENTER:return 13;
    case SDLK_NUMLOCKCLEAR:return 0x90;case SDLK_SCROLLLOCK:return 0x91;case SDLK_PRINTSCREEN:return 0x2c;
    case SDLK_LGUI:return 0x5b;case SDLK_RGUI:return 0x5c;case SDLK_APPLICATION:return 0x5d;
    default:return key<128 ? unsigned(key) : 0;
    }
}
static void Key(unsigned key,bool down,unsigned raw)
{
    if(!key) return;
    if(down) pressedKeys.insert(key);else pressedKeys.erase(key);
    const unsigned keys[9]={38,37,39,40,32,'Z','X','C','V'};
    for(unsigned i=0;i<9;++i) if(key==keys[i]) Post(0x38,0x1001+i,0,down,raw);
    Post(0x38,0x40000000+key,0,down,raw);
}
static void Pointer(unsigned kind,float x,float y,unsigned buttons)
{
    float px=x,py=y;
    if(g_sdlRenderer) SDL_RenderCoordinatesFromWindow(g_sdlRenderer,x,y,&px,&py);
    Post(0x39,kind,buttons,static_cast<unsigned>(int(px)),static_cast<unsigned>(int(py)));
}
void SdlPollEvents()
{
    SdlUpdateAudio();
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        switch(e.type) {
        case SDL_EVENT_QUIT: {
            KdEvent* event=AllocEvent();event->type=KD_EVENT_QUIT;QueueEvent(event);break;
        }
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:Post(0x2c,0,0,0,0);break;
        case SDL_EVENT_WINDOW_FOCUS_GAINED:Post(0x3c,1,0,0,0);break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            while(!pressedKeys.empty()) Key(*pressedKeys.begin(),false,0xc0000001);
            if(pointerButtons) { float x,y;SDL_GetMouseState(&x,&y);pointerButtons=0;Pointer(0x4003,x,y,0); }
            SDL_CaptureMouse(false);Post(0x3c,0,0,0,0);break;
        case SDL_EVENT_KEY_DOWN:case SDL_EVENT_KEY_UP: {
            unsigned raw=1|(unsigned(e.key.scancode)<<16)|(e.key.repeat ? 0x40000000u : 0);
            if(e.key.mod&SDL_KMOD_ALT) raw|=0x20000000u;
            if(!e.key.down) raw|=0xc0000000u;
            Key(VirtualKey(e.key.key),e.key.down,raw);break;
        }
        case SDL_EVENT_TEXT_INPUT: {
            const unsigned char* p=reinterpret_cast<const unsigned char*>(e.text.text);
            while(*p) {
                unsigned c=*p++;
                int extra=c<0x80 ? 0 : (c&0xe0)==0xc0 ? 1 : (c&0xf0)==0xe0 ? 2 : 3;
                if(extra) c&=(1u<<(6-extra))-1;
                while(extra-- && *p) c=(c<<6)|(*p++&63);
                Post(0x38,0x40000100+c,0,2,0);
            }
            break;
        }
        case SDL_EVENT_MOUSE_MOTION:Pointer(0x4001,e.motion.x,e.motion.y,pointerButtons);break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:case SDL_EVENT_MOUSE_BUTTON_UP: {
            unsigned bit=e.button.button==SDL_BUTTON_LEFT ? 1 : e.button.button==SDL_BUTTON_RIGHT ? 2 : e.button.button==SDL_BUTTON_MIDDLE ? 16 : 0;
            if(!bit) break;
            if(e.button.down) pointerButtons|=bit;else pointerButtons&=~bit;
            SDL_CaptureMouse(pointerButtons!=0);
            Pointer(0x4003,e.button.x,e.button.y,pointerButtons);break;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
            float delta=e.wheel.y*(e.wheel.direction==SDL_MOUSEWHEEL_FLIPPED ? -1 : 1);
            Pointer(0x4004,e.wheel.mouse_x,e.wheel.mouse_y,(unsigned(int(delta*120))<<16)|pointerButtons);break;
        }
        }
    }
}
}
