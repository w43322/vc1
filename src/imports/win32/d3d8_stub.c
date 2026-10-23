/* Placeholder body with the stdcall argument size of Direct3DCreate8.
   Linked into a throwaway DLL only to get an import library for the system
   d3d8.dll (see cmake/d3d8.cmake); never shipped or called. */
void* __stdcall Direct3DCreate8(unsigned sdkVersion) { return (void*)sdkVersion; }
