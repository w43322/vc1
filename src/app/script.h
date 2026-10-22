#ifndef VC1_APP_SCRIPT_H
#define VC1_APP_SCRIPT_H

namespace vc1 {
void InitScriptEngine(int argc, char** argv);
void BindHostClasses();
void InitHostServices();
void RegisterStdBindings();
// Script entry implemented by the window backend; cursor is a KD resource id.
void ScriptSetCursor(int cursor);
}
#endif
