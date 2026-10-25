#ifndef VC1_APP_OUT_OF_LINE_COPIES_H
#define VC1_APP_OUT_OF_LINE_COPIES_H

// Functions our build inlines whose bodies the original also keeps out of
// line (unused copies or callers we inline); matched byte for byte against the
// recompiled copy, relocated operands aside. Nothing includes this header.

// FUNCTION: VC1 0x004060c0
// ?AddClassificator@IClassifiable@vc1@@UAEXI@Z

// FUNCTION: VC1 0x004060d0
// ?RemoveClassificator@IClassifiable@vc1@@UAEXI@Z

// FUNCTION: VC1 0x0044dc30
// ?Read@PyroArchive@vc1@@QAEHPAXH@Z

// FUNCTION: VC1 0x0044dc40
// ??1PyroMemoryStream@vc1@@UAE@XZ

// FUNCTION: VC1 0x0044ea00
// ??0PyroLibrary@vc1@@QAE@XZ

// FUNCTION: VC1 0x0044ee20
// ??0PyroFrame@vc1@@QAE@XZ

// FUNCTION: VC1 0x0044ee90
// ??0PyroTexture@vc1@@QAE@XZ

// FUNCTION: VC1 0x0044f900
// ??1PyroCurve@vc1@@QAE@XZ

// FUNCTION: VC1 0x0044f960
// ??1PyroFrame@vc1@@QAE@XZ

// FUNCTION: VC1 0x0044f990
// ??1PyroTexture@vc1@@QAE@XZ

// FUNCTION: VC1 0x0044fa60
// ??1PyroTextureData@vc1@@QAE@XZ

// FUNCTION: VC1 0x00450120
// ??1PyroShape@vc1@@QAE@XZ

// FUNCTION: VC1 0x00453380
// ??0PyroFile@vc1@@QAE@PAVPyroLibrary@1@@Z

// FUNCTION: VC1 0x0045cf80
// ??1IGraphics@vc1@@QAE@XZ

// FUNCTION: VC1 0x00473eb0
// ??0Matrix3@vc1@@QAE@XZ

// FUNCTION: VC1 0x00496310
// ?UnlinkEvent@vc1@@YAXPAUKdEvent@1@@Z

// FUNCTION: VC1 0x00496410
// ?ReleaseHeapNode@vc1@@YAXPAUKdEvent@1@@Z

// FUNCTION: VC1 0x0049a4d0
// ?CreateFunction@SquirrelVM@@SA?AVSquirrelObject@@AAV2@P6AHPAUSQVM@@@ZPBD3@Z

// FUNCTION: VC1 0x0049b6a0
// ??1KdTSPSolver@vc1@@UAE@XZ

// FUNCTION: VC1 0x004a2920
// ??0KdTileGrid@vc1@@QAE@XZ

// FUNCTION: VC1 0x004a4610
// ??0KdTileAnimationData@vc1@@QAE@XZ

// FUNCTION: VC1 0x004a8db0
// ??0KdSystem@vc1@@QAE@XZ

// FUNCTION: VC1 0x004b8910
// ?Release@KdFactory@vc1@@QAEXXZ

// FUNCTION: VC1 0x004bb020
// ?_set_integer_slot@@YAXPAUSQVM@@PBDH@Z

// FUNCTION: VC1 0x004bc4b0
// ??1KdFileSource@vc1@@QAE@XZ

#endif
