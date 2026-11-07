// The page's way into the running game (shell/panel.js). Scripts the panel
// queues run in the game's Squirrel VM between frames, as the in-game console
// runs a typed line (KdConsole::Execute), and their results go back to the
// page. The frame wait of the d3d8 Present (vc1_web_frame) is also where the
// panel pauses the game: while paused it keeps running queued scripts and
// waiting for animation frames, but the game does not get its next frame.
#include "bridge.h"

#include <emscripten.h>
#include <windows.h>
#include <squirrel.h>
#include <string>
#include <vector>

#include "sqplus/SquirrelVM.h"

namespace {

struct Request
{
    int id;
    std::string code;
    std::string argument;
};

std::vector<Request> g_requests;
bool g_paused;

// The next animation frame (vc1_web_frame).
EM_ASYNC_JS(void, WaitForAnimationFrame, (), {
    await new Promise(function (resolve) { requestAnimationFrame(function () { resolve(); }); });
});

EM_JS(void, SendResult, (int id, int ok, const char* text), {
    if (Module.vc1Result) {
        Module.vc1Result(id, !!ok, UTF8ToString(text));
    }
});

// The value on top of the stack as text: strings as they are, anything else
// as Squirrel's tostring() has it.
std::string TopText(HSQUIRRELVM vm)
{
    const SQChar* text = 0;
    if (sq_gettype(vm, -1) != OT_STRING) {
        sq_tostring(vm, -1);
    }
    std::string result;
    if (SQ_SUCCEEDED(sq_getstring(vm, -1, &text)) && text) {
        result = text;
    }
    return result;
}

// Compiles and runs one script with the root table as "this" and
// ::vc1_arg set to its argument. Errors come back as text, not on the log.
void Run(const Request& request)
{
    HSQUIRRELVM vm = SquirrelVM::GetVMPtr();
    if (!vm) {
        SendResult(request.id, 0, "the game has not started its script VM yet");
        return;
    }
    const SQInteger top = sq_gettop(vm);
    sq_pushroottable(vm);
    sq_pushstring(vm, "vc1_arg", -1);
    sq_pushstring(vm, request.argument.c_str(), static_cast<SQInteger>(request.argument.size()));
    sq_createslot(vm, -3);
    sq_pop(vm, 1);
    const bool compiled = SQ_SUCCEEDED(sq_compilebuffer(vm, request.code.c_str(),
        static_cast<SQInteger>(request.code.size()), "panel", SQFalse));
    bool ok = compiled;
    if (ok) {
        sq_pushroottable(vm);
        ok = SQ_SUCCEEDED(sq_call(vm, 1, SQTrue, SQFalse));
    }
    std::string text;
    if (ok) {
        text = TopText(vm);
    }
    else {
        // Compile errors are marked so the page can tell them apart.
        sq_getlasterror(vm);
        text = (compiled ? "" : "compile: ") + TopText(vm);
        sq_reseterror(vm);
    }
    sq_settop(vm, top);
    SendResult(request.id, ok, text.c_str());
}

void RunRequests()
{
    std::vector<Request> requests;
    requests.swap(g_requests);
    for (size_t i = 0; i < requests.size(); ++i) {
        Run(requests[i]);
    }
}

}

extern "C" EMSCRIPTEN_KEEPALIVE void vc1_web_run(int id, const char* code, const char* argument)
{
    Request request;
    request.id = id;
    request.code = code;
    request.argument = argument;
    g_requests.push_back(request);
}

extern "C" EMSCRIPTEN_KEEPALIVE void vc1_web_pause(int paused)
{
    g_paused = paused != 0;
}

void vc1_web_frame()
{
    do {
        WaitForAnimationFrame();
        RunRequests();
    } while (g_paused);
}
