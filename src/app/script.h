#ifndef VC1_APP_SCRIPT_H
#define VC1_APP_SCRIPT_H

namespace vc1 {
void InitScriptEngine(int argc, char** argv);
void BindHostClasses();
void InitHostServices();
void RegisterStdBindings();
// Script entry in kdcore/window.cpp; cursor is a KD resource id.
void ScriptSetCursor(int cursor);
}
#endif
