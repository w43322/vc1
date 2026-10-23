#ifndef VC1_MOD_MODS_H
#define VC1_MOD_MODS_H

// Renderer-independent resource extension, not recovered original game code.
// Buffers returned by host callbacks must use malloc; the core frees them.
namespace vc1 {
struct ModImage { unsigned width, height; unsigned* pixels; };
struct ModHost {
    bool (*readFile)(const char*, unsigned char**, unsigned*);
    bool (*readPng)(const char*, ModImage*);
    bool (*tileId)(const char*, int*);
    bool (*atlasName)(int, char*, unsigned);
    void (*log)(const char*);
};
void InitMods(const char* directory, const ModHost& host);
void ShutdownMods();
bool ApplyTextureMods(const char* name, unsigned width, unsigned height, unsigned* pixels);
}
#endif
