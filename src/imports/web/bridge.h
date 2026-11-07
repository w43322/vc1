#ifndef VC1_IMPORTS_WEB_BRIDGE_H
#define VC1_IMPORTS_WEB_BRIDGE_H

// Called by the d3d8 Present after each frame (bridge.cpp): waits for the
// browser's next animation frame and runs the scripts the page queued.
void vc1_web_frame();

#endif
