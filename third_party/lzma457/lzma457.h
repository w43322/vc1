// LZMA SDK 4.57 decoder mappings. Upstream bodies are unchanged except the
// marked VC1 patch in C/7zCrc.c / C/7zCrc.h (the precomputed CRC table).
// Source: local lzma457.tar.bz2 archive, SHA-256
// e1d07b372d9748eba715354bfd540ae2cb0e2c7e83ce630439420ab7df13f2e0.
// All vendored SDK files were hash-checked against that extracted archive.
// Licensing and the static-linking exception are in lzma.txt and LGPL.txt.
// Decoder subset: CRC, 7z archive reader/extractor, LZMA and BCJ/BCJ2;
// C/Alloc.h is required by BCJ2 even though its allocator API is unused.
// Ghidra program: VirtualCity.Unwrapped.exe.
// Complete decoder symbol audit and remaining name mappings: lzma457_audit.h.
// SzAlloc/SzAllocTemp and SzFree/SzFreeTemp are identical upstream bodies,
// folded at 0x004b7100 and 0x004b70f0 respectively. These were labels only;
// official reccmp import created functions so their pseudocode could be read.
// No Ghidra writes were made through MCP.
// Low/zero asmcmp is retained rather than hidden: SzFree aliases are cdecl
// tail jumps in the original but LTCG uses EAX plus CALL/RET in this build;
// SzAlloc aliases likewise use stack input originally and EAX in this build.
// SzReadID is an 8-byte CALL wrapper originally and a 5-byte JMP here;
// asmcmp's short comparison window scores it 0 despite the same call target.
// These identities were checked with pseudocode and original raw bytes.
// Original configuration: _LZMA_IN_CB and _LZMA_PROB32, 64-bit CFileSize.
// CrcGenerateTable has no original body: the original CRC table is
// precomputed in read-only data at 0x004d06a8 (GLOBAL in C/7zCrc.c). Ghidra
// xrefs are reads in CrcUpdate/SafeReadDirectUInt32/SafeReadDirectUInt64.
// SzArDbGetFolderFullPackSize has no original standalone body identified:
// its sole SDK consumer is SzExtract's #ifndef _LZMA_IN_CB path, absent
// from original SzExtract 0x004bc6e0. No address is fabricated for it.
// AreMethodsEqual and the C++ examples in 7zIn.c/7zItem.c are commented out;
// the _LZMA_OUT_READ-only APIs are not part of this decoder configuration.

// Ghidra 0x004769d0..0x004769e8: free(Items), then zero Items/Capacity.
// ABI: ESI=buffer, free callback on stack, plain RET, no saved registers.
// Caller 0x00479f60 at 0x0047a1d2..0x0047a1dc pushes [EDI+4], sets
// ESI to the local buffer, calls this function and removes the argument.
// VC9 /GL /LTCG reccmp asm diff: 100% (2026-10-22).
// FUNCTION: VC1 0x004769d0
// SzByteBufferFree

// Ghidra 0x004769f0..0x00476a13: Capacity=size; zero capacity clears
// Items and succeeds, otherwise Items=alloc(size), return Items!=0.
// ABI: ESI=buffer, EAX=capacity, allocator on stack, plain RET; no saved
// registers. Caller SzArchiveOpen2 (0x00479f60) supplies these arguments.
// Caller 0x0047a0a8..0x0047a0b9: push [EDI], EAX=size, ESI=&buffer,
// call 0x004769f0, add ESP,4, test EAX. reccmp asm diff: 100% (2026-10-22).
// FUNCTION: VC1 0x004769f0
// SzByteBufferCreate

// Ghidra VirtualCity.Unwrapped.exe 0x00476c20..0x00476c92:
// free each coder's Properties.Items (coder stride 18), clear its buffer,
// free Coders/BindPairs/PackStreams/UnPackSizes and inline SzFolderInit.
// EDI=folder, stack freeFunc, plain RET; saves EBX/EBP/ESI, one local slot.
// Caller SzArchiveDatabaseFree 0x00476cb5..0x00476ccc sets EDI to
// Folders+i*28, pushes freeFunc, calls 0x00476c20, then ADD ESP,4.
// SzCoderInfoFree and SzFolderInit have no separate calls in this body.
// VC9 /GL /LTCG reccmp asm diff: 100% (2026-10-22).
// FUNCTION: VC1 0x00476c20
// SzFolderFree

// Ghidra VirtualCity.Unwrapped.exe 0x00476ca0..0x00476d51:
// free each folder (stride 28) and each file name (stride 20), inline
// file initialization, free five database arrays, then clear eight dwords.
// cdecl (db, freeFunc), plain RET; saves EBX/EBP/ESI/EDI.
// Caller SzArDbExFree at 0x00478e38 pushes EDI=freeFunc then ESI=db;
// caller balances the stack. SzFolderFree is called at 0x00476cc1.
// VC9 /GL /LTCG reccmp asm diff: 100% (2026-10-22).
// FUNCTION: VC1 0x00476ca0
// SzArchiveDatabaseFree

// Ghidra VirtualCity.Unwrapped.exe 0x00478e20..0x00478e67:
// free the four index arrays at +30/+34/+38/+3c, call database free
// 0x00476ca0, then inline SzArDbExInit. ESI=db, EDI=free callback,
// plain RET, no register saves; ADD ESP,18 balances six pushed arguments.
// Caller KdConstructNamed 0x0047cb00 uses the database at archive+18
// and the free callback after SzArchiveOpen2 fails (call 0x0047cc5e).
// VC9 /GL /LTCG reccmp asm diff: 100% (2026-10-22).
// FUNCTION: VC1 0x00478e20
// SzArDbExFree
