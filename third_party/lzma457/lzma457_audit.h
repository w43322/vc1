// LZMA SDK 4.57: original-function evidence and name mappings.

// Ghidra VirtualCity.Unwrapped.exe 0x4750e0..00475342.
// Upstream BranchX86_2.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int x86_2_Decode(uchar *buf0,uint size0,uchar *buf1,uint size1,uchar *buf2,uint size2,uchar *param6,
//                 uint param7,uchar *param8,uint param9)
//
// {
//   byte *pbVar1;
//   byte bVar2;
//   uint uVar3;
//   uchar *puVar4;
//   byte bVar5;
//   byte *in_EAX;
//   int iVar6;
//   int iVar7;
//   uint uVar8;
//   uchar *puVar9;
//   uchar *puVar10;
//   uint *puVar11;
//   byte local_419;
//   uint local_418;
//   uint local_414;
//   byte *local_410;
//   uint local_408 [258];
//
//   puVar11 = local_408;
//   for (iVar7 = 0x102; iVar7 != 0; iVar7 = iVar7 + -1) {
//     *puVar11 = 0x400;
//     puVar11 = puVar11 + 1;
//   }
//   pbVar1 = in_EAX + (int)param6;
//   puVar10 = (uchar *)0x0;
//   iVar7 = 0;
//   local_414 = 0;
//   local_419 = 0;
//   local_418 = 0xffffffff;
//   iVar6 = 0;
//   do {
//     if (in_EAX == pbVar1) {
//       return 1;
//     }
//     local_414 = local_414 << 8 | (uint)*in_EAX;
//     iVar6 = iVar6 + 1;
//     in_EAX = in_EAX + 1;
//   } while (iVar6 < 5);
//   local_410 = in_EAX;
//   if (param8 == (uchar *)0x0) {
//     return 0;
//   }
//   while( true ) {
//     puVar4 = puVar10;
//     uVar3 = size0 - iVar7;
//     bVar5 = local_419;
//     if ((uint)((int)param8 - (int)puVar10) < size0 - iVar7) {
//       uVar3 = (int)param8 - (int)puVar10;
//     }
//     while( true ) {
//       puVar10 = puVar4;
//       if (uVar3 == 0) goto LAB_004751ca;
//       bVar2 = buf0[iVar7];
//       puVar4[param7] = bVar2;
//       puVar10 = puVar4 + 1;
//       if (((bVar2 & 0xfe) == 0xe8) || ((bVar5 == 0xf && ((bVar2 & 0xf0) == 0x80)))) break;
//       iVar7 = iVar7 + 1;
//       puVar4 = puVar10;
//       uVar3 = uVar3 - 1;
//       bVar5 = bVar2;
//     }
//     if ((uVar3 == 0) || (puVar10 == param8)) break;
//     local_419 = buf0[iVar7];
//     iVar7 = iVar7 + 1;
//     if (local_419 == 0xe8) {
//       puVar11 = local_408 + bVar5;
//     }
//     else {
//       puVar11 = local_408 + 0x100;
//       if (local_419 != 0xe9) {
//         puVar11 = local_408 + 0x101;
//       }
//     }
//     if (local_418 < 0x1000000) {
//       if (local_410 == pbVar1) {
//         return 1;
//       }
//       local_418 = local_418 << 8;
//       local_414 = local_414 << 8 | (uint)*local_410;
//       local_410 = local_410 + 1;
//     }
//     uVar3 = *puVar11;
//     uVar8 = (local_418 >> 0xb) * uVar3;
//     if (local_414 < uVar8) {
//       *puVar11 = (0x800 - uVar3 >> 5) + uVar3;
//       local_418 = uVar8;
//     }
//     else {
//       local_418 = local_418 - uVar8;
//       local_414 = local_414 - uVar8;
//       *puVar11 = uVar3 - (uVar3 >> 5);
//       if (local_419 == 0xe8) {
//         if (size1 < 4) {
//           return 1;
//         }
//         size1 = size1 - 4;
//         puVar9 = buf1;
//         buf1 = buf1 + 4;
//       }
//       else {
//         if (size2 < 4) {
//           return 1;
//         }
//         size2 = size2 - 4;
//         puVar9 = buf2;
//         buf2 = buf2 + 4;
//       }
//       iVar6 = (CONCAT31(CONCAT21(CONCAT11(*puVar9,puVar9[1]),puVar9[2]),puVar9[3]) - (int)puVar10) +
//               -4;
//       puVar10[param7] = (uchar)iVar6;
//       puVar10 = puVar4 + 2;
//       if (puVar10 == param8) break;
//       puVar10[param7] = (uchar)((uint)iVar6 >> 8);
//       puVar10 = puVar4 + 3;
//       if (puVar10 == param8) break;
//       puVar10[param7] = (uchar)((uint)iVar6 >> 0x10);
//       puVar10 = puVar4 + 4;
//       if (puVar10 == param8) break;
//       local_419 = (byte)((uint)iVar6 >> 0x18);
//       puVar10[param7] = local_419;
//       puVar10 = puVar4 + 5;
//     }
//   }
// LAB_004751ca:
//   return (uint)(puVar10 != param8);
// }
// ABI / bounds, original assembly:
// 004750e0 SUB ESP,0x41c
// 004750e6 PUSH EBX
// 004750e7 PUSH EBP
// 004750e8 PUSH ESI
// 004750e9 MOV ESI,EAX
// 004750eb PUSH EDI
// 004750ec MOV EAX,0x400
// 0047532a CMP EDX,dword ptr [ESP + 0x450]
// 00475331 JZ 0x004751ca
// 00475337 SHR EAX,0x18
// 0047533a MOV byte ptr [EDX + ESI*0x1],AL
// 0047533d MOV byte ptr [ESP + 0x13],AL
// 00475341 INC EDX
// 00475342 JMP 0x00475167
// Incoming reference SzDecode2:
// 004767e4 PUSH EDX
// 004767e5 MOV EDX,dword ptr [ESP + 0x34]
// 004767e9 PUSH EAX
// 004767ea MOV EAX,dword ptr [EBX + 0x8]
// 004767ed PUSH ECX
// 004767ee PUSH EDX
// 004767ef CALL 0x004750e0
// 004767f4 ADD ESP,0x24
// 004767f7 JMP 0x0047691b
// 004767fc MOV ECX,dword ptr [ESP + 0x5c]
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x4750e0
// x86_2_Decode

// Ghidra VirtualCity.Unwrapped.exe 0x475350..0047607d.
// Upstream LzmaDecode.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int LzmaDecode(_CLzmaDecoderState *vs,_ILzmaInCallback *InCallback,uchar *outStream,uint outSize,
//               uint *outSizeProcessed)
//
// {
//   uint *puVar1;
//   int iVar2;
//   uint uVar3;
//   byte *pbVar4;
//   byte bVar5;
//   uint uVar6;
//   uint uVar7;
//   int iVar8;
//   uint uVar9;
//   uint *puVar10;
//   uint uVar11;
//   uint uVar12;
//   uint uVar13;
//   byte local_79;
//   byte *local_78;
//   byte *local_74;
//   uint local_70;
//   uint local_6c;
//   uint local_68;
//   uint *local_64;
//   uint *puStack_60;
//   uint local_5c;
//   uint local_58;
//   uint local_54;
//   uint uStack_50;
//   int local_4c;
//   uint *puStack_48;
//   uint *local_44;
//   uint local_40;
//   int iStack_3c;
//   uint local_38;
//   int iStack_34;
//   int iStack_30;
//   int iStack_2c;
//   int iStack_28;
//   int iStack_24;
//   int iStack_20;
//   int iStack_1c;
//   int iStack_18;
//   int iStack_14;
//   int iStack_10;
//   int iStack_c;
//   int iStack_8;
//   int iStack_4;
//
//   local_64 = vs->Probs;
//   local_4c = (vs->Properties).lc;
//   local_40 = (1 << ((byte)(vs->Properties).pb & 0x1f)) - 1;
//   bVar5 = (byte)(vs->Properties).lp;
//   local_38 = (1 << (bVar5 & 0x1f)) - 1;
//   iVar8 = 0;
//   iVar2 = (0x300 << (bVar5 + (char)local_4c & 0x1f)) + 0x736;
//   local_6c = 0;
//   local_79 = 0;
//   local_68 = 0;
//   local_70 = 1;
//   local_58 = 1;
//   local_5c = 1;
//   local_54 = 1;
//   *outSizeProcessed = 0;
//   puVar10 = local_64;
//   if (iVar2 != 0) {
//     for (; iVar2 != 0; iVar2 = iVar2 + -1) {
//       *puVar10 = 0x400;
//       puVar10 = puVar10 + 1;
//     }
//   }
//   uVar13 = 0;
//   local_74 = (byte *)0x0;
//   local_78 = (byte *)0x0;
//   uVar11 = 0xffffffff;
//   do {
//     if (local_78 == local_74) {
//       iVar2 = (*InCallback->Read)(InCallback,&local_78,&local_44);
//       if (iVar2 != 0) {
//         return iVar2;
//       }
//       local_74 = (byte *)((int)local_44 + (int)local_78);
//       if (local_44 == (uint *)0x0) {
//         return 1;
//       }
//     }
//     bVar5 = *local_78;
//     local_78 = local_78 + 1;
//     iVar8 = iVar8 + 1;
//     uVar13 = uVar13 << 8 | (uint)bVar5;
//   } while (iVar8 < 5);
//   if (outSize != 0) {
// LAB_00475454:
//     uVar9 = local_68;
//     uStack_50 = local_40 & local_6c;
//     puVar10 = local_64 + local_68 * 0x10 + uStack_50;
//     uVar12 = uVar11;
//     puStack_60 = puVar10;
//     if (uVar11 < 0x1000000) {
//       if (local_78 == local_74) {
//         iVar2 = (*InCallback->Read)(InCallback,&local_78,&iStack_3c);
//         if (iVar2 != 0) {
//           return iVar2;
//         }
//         local_74 = local_78 + iStack_3c;
//         if (iStack_3c == 0) {
//           return 1;
//         }
//       }
//       uVar12 = uVar11 << 8;
//       uVar13 = uVar13 << 8 | (uint)*local_78;
//       local_78 = local_78 + 1;
//     }
//     uVar6 = *puVar10;
//     uVar11 = (uVar12 >> 0xb) * uVar6;
//     if (uVar13 < uVar11) {
//       *puStack_60 = (0x800 - uVar6 >> 5) + uVar6;
//       iVar2 = 1;
//       puVar10 = local_64 +
//                 ((uint)(local_79 >> (8 - (byte)local_4c & 0x1f)) +
//                 ((local_38 & local_6c) << ((byte)local_4c & 0x1f))) * 0x300 + 0x736;
//       puStack_60 = puVar10;
//       if ((int)uVar9 < 7) goto LAB_00475610;
//       uStack_50 = (uint)outStream[local_6c - local_70];
//       do {
//         uStack_50 = uStack_50 * 2;
//         uVar9 = uStack_50 & 0x100;
//         local_44 = puStack_60 + iVar2 + uVar9 + 0x100;
//         if (uVar11 < 0x1000000) {
//           if (local_78 == local_74) {
//             iVar8 = (*InCallback->Read)(InCallback,&local_78,&iStack_34);
//             if (iVar8 != 0) {
//               return iVar8;
//             }
//             local_74 = local_78 + iStack_34;
//             if (iStack_34 == 0) {
//               return 1;
//             }
//           }
//           uVar11 = uVar11 << 8;
//           uVar13 = uVar13 << 8 | (uint)*local_78;
//           local_78 = local_78 + 1;
//         }
//         uVar12 = *local_44;
//         uVar6 = (uVar11 >> 0xb) * uVar12;
//         puVar10 = puStack_60;
//         if (uVar13 < uVar6) {
//           iVar2 = iVar2 * 2;
//           *local_44 = (0x800 - uVar12 >> 5) + uVar12;
//           uVar11 = uVar6;
//           if (uVar9 != 0) goto joined_r0x00475602;
//         }
//         else {
//           uVar11 = uVar11 - uVar6;
//           uVar13 = uVar13 - uVar6;
//           *local_44 = uVar12 - (uVar12 >> 5);
//           iVar2 = iVar2 * 2 + 1;
//           if (uVar9 == 0) goto joined_r0x00475602;
//         }
//       } while (iVar2 < 0x100);
//       goto LAB_004756d7;
//     }
//     uVar12 = uVar12 - uVar11;
//     uVar13 = uVar13 - uVar11;
//     *puVar10 = uVar6 - (uVar6 >> 5);
//     if (uVar12 < 0x1000000) {
//       if (local_78 == local_74) {
//         iVar2 = (*InCallback->Read)(InCallback,&local_78,&iStack_2c);
//         if (iVar2 != 0) {
//           return iVar2;
//         }
//         local_74 = local_78 + iStack_2c;
//         if (iStack_2c == 0) {
//           return 1;
//         }
//       }
//       uVar12 = uVar12 * 0x100;
//       uVar13 = uVar13 * 0x100 | (uint)*local_78;
//       local_78 = local_78 + 1;
//     }
//     uVar11 = local_64[uVar9 + 0xc0];
//     uVar6 = (uVar12 >> 0xb) * uVar11;
//     if (uVar6 <= uVar13) {
//       uVar12 = uVar12 - uVar6;
//       uVar13 = uVar13 - uVar6;
//       local_64[uVar9 + 0xc0] = uVar11 - (uVar11 >> 5);
//       if (uVar12 < 0x1000000) {
//         if (local_78 == local_74) {
//           iVar2 = (*InCallback->Read)(InCallback,&local_78,&iStack_28);
//           if (iVar2 != 0) {
//             return iVar2;
//           }
//           local_74 = local_78 + iStack_28;
//           if (iStack_28 == 0) {
//             return 1;
//           }
//         }
//         uVar12 = uVar12 * 0x100;
//         uVar13 = uVar13 * 0x100 | (uint)*local_78;
//         local_78 = local_78 + 1;
//       }
//       uVar11 = local_64[uVar9 + 0xcc];
//       uVar6 = (uVar12 >> 0xb) * uVar11;
//       if (uVar6 <= uVar13) {
//         uVar12 = uVar12 - uVar6;
//         uVar13 = uVar13 - uVar6;
//         local_64[uVar9 + 0xcc] = uVar11 - (uVar11 >> 5);
//         if (uVar12 < 0x1000000) {
//           if (local_78 == local_74) {
//             iVar2 = (*InCallback->Read)(InCallback,&local_78,&iStack_20);
//             if (iVar2 != 0) {
//               return iVar2;
//             }
//             local_74 = local_78 + iStack_20;
//             if (iStack_20 == 0) {
//               return 1;
//             }
//           }
//           uVar12 = uVar12 * 0x100;
//           uVar13 = uVar13 * 0x100 | (uint)*local_78;
//           local_78 = local_78 + 1;
//         }
//         uVar11 = local_64[uVar9 + 0xd8];
//         uVar6 = (uVar12 >> 0xb) * uVar11;
//         if (uVar13 < uVar6) {
//           local_64[uVar9 + 0xd8] = (0x800 - uVar11 >> 5) + uVar11;
//           uVar3 = local_58;
//         }
//         else {
//           uVar12 = uVar12 - uVar6;
//           uVar13 = uVar13 - uVar6;
//           local_64[uVar9 + 0xd8] = uVar11 - (uVar11 >> 5);
//           if (uVar12 < 0x1000000) {
//             if (local_78 == local_74) {
//               iVar2 = (*InCallback->Read)(InCallback,&local_78,&iStack_1c);
//               if (iVar2 != 0) {
//                 return iVar2;
//               }
//               local_74 = local_78 + iStack_1c;
//               if (iStack_1c == 0) {
//                 return 1;
//               }
//             }
//             uVar12 = uVar12 * 0x100;
//             uVar13 = uVar13 * 0x100 | (uint)*local_78;
//             local_78 = local_78 + 1;
//           }
//           uVar3 = local_54;
//           uVar11 = local_64[uVar9 + 0xe4];
//           uVar7 = (uVar12 >> 0xb) * uVar11;
//           if (uVar13 < uVar7) {
//             local_64[uVar9 + 0xe4] = (0x800 - uVar11 >> 5) + uVar11;
//             uVar3 = local_5c;
//             uVar6 = uVar7;
//           }
//           else {
//             uVar6 = uVar12 - uVar7;
//             uVar13 = uVar13 - uVar7;
//             local_64[uVar9 + 0xe4] = uVar11 - (uVar11 >> 5);
//             local_54 = local_5c;
//           }
//           local_5c = local_58;
//         }
//         local_58 = local_70;
//         local_70 = uVar3;
// LAB_00475b51:
//         local_68 = ((6 < (int)uVar9) - 1 & 0xfffffffd) + 0xb;
//         puVar10 = local_64 + 0x534;
//         goto LAB_00475b6e;
//       }
//       local_64[uVar9 + 0xcc] = (0x800 - uVar11 >> 5) + uVar11;
//       puStack_60 = local_64 + (uVar9 + 0xf) * 0x10 + uStack_50;
//       if (uVar6 < 0x1000000) {
//         if (local_78 == local_74) {
//           iVar2 = (*InCallback->Read)(InCallback,&local_78,&iStack_24);
//           if (iVar2 != 0) {
//             return iVar2;
//           }
//           local_74 = local_78 + iStack_24;
//           if (iStack_24 == 0) {
//             return 1;
//           }
//         }
//         uVar6 = uVar6 * 0x100;
//         uVar13 = uVar13 << 8 | (uint)*local_78;
//         local_78 = local_78 + 1;
//       }
//       uVar12 = *puStack_60;
//       uVar11 = (uVar6 >> 0xb) * uVar12;
//       if (uVar11 <= uVar13) {
//         uVar6 = uVar6 - uVar11;
//         uVar13 = uVar13 - uVar11;
//         *puStack_60 = uVar12 - (uVar12 >> 5);
//         goto LAB_00475b51;
//       }
//       *puStack_60 = (0x800 - uVar12 >> 5) + uVar12;
//       if (local_6c == 0) {
//         return 1;
//       }
//       local_68 = (uint)(6 < (int)uVar9) * 2 + 9;
//       local_79 = outStream[local_6c - local_70];
//       outStream[local_6c] = local_79;
//       local_6c = local_6c + 1;
//       goto LAB_00475711;
//     }
//     local_54 = local_5c;
//     local_64[uVar9 + 0xc0] = (0x800 - uVar11 >> 5) + uVar11;
//     local_5c = local_58;
//     local_58 = local_70;
//     local_68 = ((int)uVar9 < 7) - 1 & 3;
//     puVar10 = local_64 + 0x332;
// LAB_00475b6e:
//     if (uVar6 < 0x1000000) {
//       if (local_78 == local_74) {
//         iVar2 = (*InCallback->Read)(InCallback,&local_78,&iStack_18);
//         if (iVar2 != 0) {
//           return iVar2;
//         }
//         local_74 = local_78 + iStack_18;
//         if (iStack_18 == 0) {
//           return 1;
//         }
//       }
//       uVar6 = uVar6 << 8;
//       uVar13 = uVar13 << 8 | (uint)*local_78;
//       local_78 = local_78 + 1;
//     }
//     uVar9 = *puVar10;
//     uVar11 = (uVar6 >> 0xb) * uVar9;
//     if (uVar13 < uVar11) {
//       *puVar10 = (0x800 - uVar9 >> 5) + uVar9;
//       puVar10 = puVar10 + uStack_50 * 8 + 2;
//       uVar9 = 0;
//       puStack_60 = (uint *)0x3;
//     }
//     else {
//       uVar6 = uVar6 - uVar11;
//       uVar13 = uVar13 - uVar11;
//       *puVar10 = uVar9 - (uVar9 >> 5);
//       if (uVar6 < 0x1000000) {
//         if (local_78 == local_74) {
//           iVar2 = (*InCallback->Read)(InCallback,&local_78,&iStack_14);
//           if (iVar2 != 0) {
//             return iVar2;
//           }
//           local_74 = local_78 + iStack_14;
//           if (iStack_14 == 0) {
//             return 1;
//           }
//         }
//         uVar6 = uVar6 * 0x100;
//         uVar13 = uVar13 * 0x100 | (uint)*local_78;
//         local_78 = local_78 + 1;
//       }
//       uVar9 = puVar10[1];
//       uVar12 = (uVar6 >> 0xb) * uVar9;
//       if (uVar13 < uVar12) {
//         puVar10[1] = (0x800 - uVar9 >> 5) + uVar9;
//         puVar10 = puVar10 + uStack_50 * 8 + 0x82;
//         uVar9 = 8;
//         puStack_60 = (uint *)0x3;
//         uVar11 = uVar12;
//       }
//       else {
//         uVar11 = uVar6 - uVar12;
//         uVar13 = uVar13 - uVar12;
//         puVar10[1] = uVar9 - (uVar9 >> 5);
//         puVar10 = puVar10 + 0x102;
//         uVar9 = 0x10;
//         puStack_60 = (uint *)0x8;
//       }
//     }
//     puStack_48 = puStack_60;
//     iVar2 = 1;
//     uStack_50 = uVar9;
//     do {
//       if (uVar11 < 0x1000000) {
//         if (local_78 == local_74) {
//           iVar8 = (*InCallback->Read)(InCallback,&local_78,&iStack_10);
//           if (iVar8 != 0) {
//             return iVar8;
//           }
//           local_74 = local_78 + iStack_10;
//           if (iStack_10 == 0) {
//             return 1;
//           }
//         }
//         uVar11 = uVar11 << 8;
//         uVar13 = uVar13 << 8 | (uint)*local_78;
//         local_78 = local_78 + 1;
//         uVar9 = uStack_50;
//       }
//       puVar1 = local_64;
//       uVar12 = puVar10[iVar2];
//       uVar6 = (uVar11 >> 0xb) * uVar12;
//       if (uVar13 < uVar6) {
//         puVar10[iVar2] = (0x800 - uVar12 >> 5) + uVar12;
//         iVar2 = iVar2 * 2;
//         uVar11 = uVar6;
//       }
//       else {
//         uVar11 = uVar11 - uVar6;
//         uVar13 = uVar13 - uVar6;
//         puVar10[iVar2] = uVar12 - (uVar12 >> 5);
//         iVar2 = iVar2 * 2 + 1;
//       }
//       puStack_48 = (uint *)((int)puStack_48 + -1);
//     } while (puStack_48 != (uint *)0x0);
//     local_44 = (uint *)(iVar2 + (uVar9 - (1 << ((byte)puStack_60 & 0x1f))));
//     puStack_48 = (uint *)0x0;
//     if ((int)local_68 < 4) {
//       local_68 = local_68 + 7;
//       puVar10 = local_44;
//       if (3 < (int)local_44) {
//         puVar10 = (uint *)0x3;
//       }
//       puStack_48 = (uint *)0x6;
//       iVar2 = 1;
//       do {
//         if (uVar11 < 0x1000000) {
//           if (local_78 == local_74) {
//             iVar8 = (*InCallback->Read)(InCallback,&local_78,&iStack_c);
//             if (iVar8 != 0) {
//               return iVar8;
//             }
//             local_74 = local_78 + iStack_c;
//             if (iStack_c == 0) {
//               return 1;
//             }
//           }
//           uVar11 = uVar11 << 8;
//           uVar13 = uVar13 << 8 | (uint)*local_78;
//           local_78 = local_78 + 1;
//         }
//         uVar9 = puVar1[(int)puVar10 * 0x40 + iVar2 + 0x1b0];
//         uVar12 = (uVar11 >> 0xb) * uVar9;
//         if (uVar13 < uVar12) {
//           puVar1[(int)puVar10 * 0x40 + iVar2 + 0x1b0] = (0x800 - uVar9 >> 5) + uVar9;
//           iVar2 = iVar2 * 2;
//           uVar11 = uVar12;
//         }
//         else {
//           uVar11 = uVar11 - uVar12;
//           uVar13 = uVar13 - uVar12;
//           puVar1[(int)puVar10 * 0x40 + iVar2 + 0x1b0] = uVar9 - (uVar9 >> 5);
//           iVar2 = iVar2 * 2 + 1;
//         }
//         puStack_48 = (uint *)((int)puStack_48 + -1);
//       } while (puStack_48 != (uint *)0x0);
//       uVar9 = iVar2 - 0x40;
//       puStack_48 = (uint *)0x0;
//       local_70 = uVar9;
//       if (3 < (int)uVar9) {
//         puStack_60 = (uint *)(((int)uVar9 >> 1) + -1);
//         local_70 = uVar9 & 1 | 2;
//         pbVar4 = local_78;
//         if ((int)uVar9 < 0xe) {
//           local_70 = local_70 << ((byte)puStack_60 & 0x1f);
//           uStack_50 = 1;
//           puVar10 = local_64 + (local_70 - uVar9) + 0x2af;
//           iVar2 = 1;
//         }
//         else {
//           puStack_60 = (uint *)(((int)uVar9 >> 1) + -5);
//           do {
//             uVar9 = local_70;
//             if (uVar11 < 0x1000000) {
//               if (pbVar4 == local_74) {
//                 iVar2 = (*InCallback->Read)(InCallback,&local_78,&iStack_8);
//                 if (iVar2 != 0) {
//                   return iVar2;
//                 }
//                 local_74 = local_78 + iStack_8;
//                 pbVar4 = local_78;
//                 if (iStack_8 == 0) {
//                   return 1;
//                 }
//               }
//               uVar11 = uVar11 << 8;
//               uVar13 = uVar13 << 8 | (uint)*pbVar4;
//               pbVar4 = pbVar4 + 1;
//               local_78 = pbVar4;
//             }
//             uVar11 = uVar11 >> 1;
//             local_70 = uVar9 * 2;
//             if (uVar11 <= uVar13) {
//               uVar13 = uVar13 - uVar11;
//               local_70 = local_70 | 1;
//             }
//             puStack_60 = (uint *)((int)puStack_60 + -1);
//           } while (puStack_60 != (uint *)0x0);
//           puVar10 = local_64 + 0x322;
//           local_70 = local_70 << 4;
//           puStack_60 = (uint *)0x4;
//           uStack_50 = 1;
//           iVar2 = 1;
//         }
//         do {
//           if (uVar11 < 0x1000000) {
//             if (pbVar4 == local_74) {
//               iVar8 = (*InCallback->Read)(InCallback,&local_78,&iStack_4);
//               if (iVar8 != 0) {
//                 return iVar8;
//               }
//               local_74 = local_78 + iStack_4;
//               pbVar4 = local_78;
//               if (iStack_4 == 0) {
//                 return 1;
//               }
//             }
//             uVar11 = uVar11 << 8;
//             uVar13 = uVar13 << 8 | (uint)*pbVar4;
//             local_78 = pbVar4 + 1;
//           }
//           uVar9 = puVar10[iVar2];
//           uVar12 = (uVar11 >> 0xb) * uVar9;
//           if (uVar13 < uVar12) {
//             puVar10[iVar2] = (0x800 - uVar9 >> 5) + uVar9;
//             iVar2 = iVar2 * 2;
//             uVar11 = uVar12;
//           }
//           else {
//             uVar11 = uVar11 - uVar12;
//             uVar13 = uVar13 - uVar12;
//             local_70 = local_70 | uStack_50;
//             puVar10[iVar2] = uVar9 - (uVar9 >> 5);
//             iVar2 = iVar2 * 2 + 1;
//           }
//           uStack_50 = uStack_50 << 1;
//           puStack_60 = (uint *)((int)puStack_60 + -1);
//           pbVar4 = local_78;
//         } while (puStack_60 != (uint *)0x0);
//         puStack_60 = (uint *)0x0;
//       }
//       local_70 = local_70 + 1;
//       if (local_70 == 0) goto LAB_00475722;
//     }
//     iVar2 = (int)local_44 + 2;
//     if (local_6c < local_70) {
//       return 1;
//     }
//     pbVar4 = outStream + (local_6c - local_70);
//     while( true ) {
//       local_79 = *pbVar4;
//       iVar2 = iVar2 + -1;
//       outStream[local_6c] = local_79;
//       local_6c = local_6c + 1;
//       pbVar4 = pbVar4 + 1;
//       if (iVar2 == 0) break;
//       if (outSize <= local_6c) goto LAB_00475722;
//     }
//     goto LAB_00475711;
//   }
//   goto LAB_0047576f;
// joined_r0x00475602:
//   while (iVar2 < 0x100) {
// LAB_00475610:
//     if (uVar11 < 0x1000000) {
//       if (local_78 == local_74) {
//         iVar8 = (*InCallback->Read)(InCallback,&local_78,&iStack_30);
//         if (iVar8 != 0) {
//           return iVar8;
//         }
//         local_74 = local_78 + iStack_30;
//         if (iStack_30 == 0) {
//           return 1;
//         }
//       }
//       uVar11 = uVar11 << 8;
//       uVar13 = uVar13 << 8 | (uint)*local_78;
//       local_78 = local_78 + 1;
//       puVar10 = puStack_60;
//     }
//     uVar9 = puVar10[iVar2];
//     uVar12 = (uVar11 >> 0xb) * uVar9;
//     if (uVar13 < uVar12) {
//       puVar10[iVar2] = (0x800 - uVar9 >> 5) + uVar9;
//       iVar2 = iVar2 * 2;
//       uVar11 = uVar12;
//     }
//     else {
//       uVar11 = uVar11 - uVar12;
//       uVar13 = uVar13 - uVar12;
//       puVar10[iVar2] = uVar9 - (uVar9 >> 5);
//       iVar2 = iVar2 * 2 + 1;
//     }
//   }
// LAB_004756d7:
//   local_79 = (byte)iVar2;
//   outStream[local_6c] = local_79;
//   local_6c = local_6c + 1;
//   if ((int)local_68 < 4) {
//     local_68 = 0;
//   }
//   else if ((int)local_68 < 10) {
//     local_68 = local_68 - 3;
//   }
//   else {
//     local_68 = local_68 - 6;
//   }
// LAB_00475711:
//   if (outSize <= local_6c) goto LAB_00475722;
//   goto LAB_00475454;
// LAB_00475722:
//   if (uVar11 < 0x1000000) {
//     if (local_78 == local_74) {
//       iVar2 = (*InCallback->Read)(InCallback,&local_78,&iStack_4);
//       if (iVar2 != 0) {
//         return iVar2;
//       }
//       local_74 = local_78 + iStack_4;
//       if (iStack_4 == 0) {
//         return 1;
//       }
//     }
//     local_78 = local_78 + 1;
//   }
// LAB_0047576f:
//   vs->Buffer = local_78;
//   vs->BufferLim = local_74;
//   *outSizeProcessed = local_6c;
//   return 0;
// }
// ABI / bounds, original assembly:
// 00475350 SUB ESP,0x7c
// 00475353 MOV EAX,dword ptr [ESP + 0x80]
// 0047535a MOV ECX,dword ptr [EAX + 0x8]
// 0047535d PUSH EBX
// 0047535e PUSH EBP
// 0047535f MOV EBP,0x1
// 00475364 MOV EDX,EBP
// 00476071 POP EDI
// 00476072 POP ESI
// 00476073 POP EBP
// 00476074 MOV EAX,0x1
// 00476079 POP EBX
// 0047607a ADD ESP,0x7c
// 0047607d RET
// Incoming reference SzDecodeLzma:
// 0047664a PUSH EBP
// 0047664b PUSH EAX
// 0047664c LEA ECX,[ESP + 0x1c]
// 00476650 PUSH ECX
// 00476651 LEA EDX,[ESP + 0x30]
// 00476655 PUSH EDX
// 00476656 CALL 0x00475350
// 0047665b MOV ECX,dword ptr [EBX + 0x4]
// 0047665e MOV ESI,EAX
// 00476660 MOV EAX,dword ptr [ESP + 0x40]
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x475350
// LzmaDecode

// Ghidra VirtualCity.Unwrapped.exe 0x476080..004760f2.
// Upstream LzmaDecode.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int LzmaDecodeProperties(_CLzmaProperties *param0,uchar *param1,int param2)
//
// {
//   byte *in_EAX;
//   uint uVar1;
//   byte bVar2;
//   uint uVar3;
//   uint *unaff_EDI;
//
//   bVar2 = *in_EAX;
//   if (0xe0 < bVar2) {
//     return 1;
//   }
//   unaff_EDI[2] = 0;
//   if (0x2c < bVar2) {
//     uVar3 = (uint)bVar2;
//     uVar1 = uVar3 / 0x2d;
//     do {
//       bVar2 = bVar2 - 0x2d;
//       uVar1 = uVar1 - 1;
//     } while (uVar1 != 0);
//     unaff_EDI[2] = uVar3 / 0x2d;
//   }
//   unaff_EDI[1] = 0;
//   if (8 < bVar2) {
//     uVar1 = (uint)bVar2;
//     uVar3 = uVar1 / 9;
//     do {
//       bVar2 = bVar2 - 9;
//       uVar3 = uVar3 - 1;
//     } while (uVar3 != 0);
//     unaff_EDI[1] = uVar1 / 9;
//   }
//   *unaff_EDI = (uint)bVar2;
//   return 0;
// }
// ABI / bounds, original assembly:
// 00476080 MOV CL,byte ptr [EAX]
// 00476082 CMP CL,0xe1
// 00476085 JC 0x0047608d
// 00476087 MOV EAX,0x1
// 0047608c RET
// 0047608d MOV dword ptr [EDI + 0x8],0x0
// 00476094 CMP CL,0x2d
// 004760e3 SUB EAX,0x1
// 004760e6 JNZ 0x004760e0
// 004760e8 MOV dword ptr [EDI + 0x4],EDX
// 004760eb MOVZX EAX,CL
// 004760ee MOV dword ptr [EDI],EAX
// 004760f0 XOR EAX,EAX
// 004760f2 RET
// Incoming reference SzDecodeLzma:
// 004765de MOV dword ptr [ESP + 0x1c],EDX
// 004765e2 MOV dword ptr [ESP + 0x14],ECX
// 004765e6 MOV dword ptr [ESP + 0x10],0x4764c0
// 004765ee JL 0x00476600
// 004765f0 MOV EAX,dword ptr [EAX + 0x14]
// 004765f3 LEA EDI,[ESP + 0x20]
// 004765f7 CALL 0x00476080
// 004765fc TEST EAX,EAX
// 004765fe JZ 0x0047660c
// 00476600 MOV EAX,0x80004005
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x476080
// LzmaDecodeProperties

// Ghidra VirtualCity.Unwrapped.exe 0x476100..00476281.
// Upstream BranchX86.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// uint x86_Convert(uchar *buffer,uint param1,uint param2,uint *param3,int param4)
//
// {
//   byte bVar1;
//   uint in_EAX;
//   uint uVar2;
//   char cVar3;
//   int iVar4;
//   uint uVar5;
//   uint uVar6;
//   byte *pbVar7;
//   uint local_8;
//
//   iVar4 = 0;
//   uVar6 = *(uint *)param1 & 7;
//   if (in_EAX < 5) {
//     return 0;
//   }
//   local_8 = 0xffffffff;
//   do {
//     while( true ) {
//       for (pbVar7 = buffer + iVar4; (pbVar7 < buffer + (in_EAX - 4) && ((*pbVar7 & 0xfe) != 0xe8));
//           pbVar7 = pbVar7 + 1) {
//       }
//       uVar5 = (int)pbVar7 - (int)buffer;
//       uVar2 = uVar5 - local_8;
//       if (buffer + (in_EAX - 4) <= pbVar7) {
//         if (3 < uVar2) {
//           *(undefined4 *)param1 = 0;
//           return uVar5;
//         }
//         *(uint *)param1 = uVar6 << ((char)uVar2 - 1U & 0x1f) & 7;
//         return uVar5;
//       }
//       local_8 = uVar5;
//       if (uVar2 < 4) break;
//       uVar6 = 0;
// LAB_00476166:
//       bVar1 = pbVar7[4];
//       if ((bVar1 == 0) || (bVar1 == 0xff)) {
//         uVar2 = CONCAT31(CONCAT21(CONCAT11(bVar1,pbVar7[3]),pbVar7[2]),pbVar7[1]);
//         while (uVar2 = (uVar2 - uVar5) - 5, uVar6 != 0) {
//           cVar3 = (char)(uVar2 >> ((&DAT_004d069c)[uVar6] * -8 + 0x18U & 0x1f));
//           if ((cVar3 != '\0') && (cVar3 != -1)) break;
//           uVar2 = (1 << ((&DAT_004d069c)[uVar6] * -8 + 0x20U & 0x1f)) - 1U ^ uVar2;
//         }
//         pbVar7[4] = ~(((byte)(uVar2 >> 0x18) & 1) - 1);
//         pbVar7[2] = (byte)(uVar2 >> 8);
//         pbVar7[3] = (byte)(uVar2 >> 0x10);
//         pbVar7[1] = (byte)uVar2;
//         iVar4 = uVar5 + 5;
//       }
//       else {
//         uVar6 = (uVar6 & 3) * 2 | 1;
//         iVar4 = uVar5 + 1;
//       }
//     }
//     uVar2 = uVar6 << ((char)uVar2 - 1U & 0x1f);
//     uVar6 = uVar2 & 7;
//     if ((uVar6 == 0) ||
//        ((((&DAT_004d0694)[uVar6] != '\0' && (pbVar7[4 - (uint)(byte)(&DAT_004d069c)[uVar6]] != 0))
//         && (pbVar7[4 - (uint)(byte)(&DAT_004d069c)[uVar6]] != 0xff)))) goto LAB_00476166;
//     uVar6 = (uVar2 & 3) * 2 | 1;
//     iVar4 = uVar5 + 1;
//   } while( true );
// }
// ABI / bounds, original assembly:
// 00476100 MOV ECX,dword ptr [ESP + 0x8]
// 00476104 SUB ESP,0x8
// 00476107 PUSH EBP
// 00476108 PUSH ESI
// 00476109 MOV ESI,dword ptr [ECX]
// 0047610b XOR EBP,EBP
// 0047610d AND ESI,0x7
// 00476275 AND ESI,0x7
// 00476278 MOV dword ptr [EAX],ESI
// 0047627a POP ESI
// 0047627b MOV EAX,EBP
// 0047627d POP EBP
// 0047627e ADD ESP,0x8
// 00476281 RET
// Incoming reference SzDecode2:
// 00476712 MOV EAX,dword ptr [ESP + 0x58]
// 00476716 LEA EDX,[ESP + 0x18]
// 0047671a PUSH EDX
// 0047671b PUSH EAX
// 0047671c MOV EAX,dword ptr [ESP + 0x64]
// 00476720 MOV dword ptr [ESP + 0x20],EBP
// 00476724 CALL 0x00476100
// 00476729 ADD ESP,0x8
// 0047672c JMP 0x00476929
// 00476731 CMP ECX,0x303011b
// VC9 /GL /LTCG asmcmp 2026-10-22: 97.95%.
// FUNCTION: VC1 0x476100
// x86_Convert

// Ghidra VirtualCity.Unwrapped.exe 0x476290..00476305.
// Upstream 7zDecode.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// ulonglong GetSum(ulonglong *param0,uint param1)
//
// {
//   uint uVar1;
//   uint uVar2;
//   ulonglong *puVar3;
//   int iVar4;
//   int iVar5;
//   int unaff_ESI;
//   uint uVar6;
//   bool bVar7;
//
//   uVar2 = 0;
//   uVar6 = 0;
//   iVar5 = 0;
//   iVar4 = 0;
//   puVar3 = (ulonglong *)0x0;
//   if (1 < (int)param0) {
//     do {
//       uVar1 = *(uint *)(unaff_ESI + (int)puVar3 * 8);
//       bVar7 = CARRY4(uVar6,uVar1);
//       uVar6 = uVar6 + uVar1;
//       iVar5 = iVar5 + *(int *)(unaff_ESI + 4 + (int)puVar3 * 8) + (uint)bVar7;
//       uVar1 = *(uint *)(unaff_ESI + 8 + (int)puVar3 * 8);
//       bVar7 = CARRY4(uVar2,uVar1);
//       uVar2 = uVar2 + uVar1;
//       iVar4 = iVar4 + *(int *)(unaff_ESI + 0xc + (int)puVar3 * 8) + (uint)bVar7;
//       puVar3 = (ulonglong *)((int)puVar3 + 2);
//     } while (puVar3 < (ulonglong *)((int)param0 - 1U));
//   }
//   if (puVar3 < param0) {
//     uVar1 = *(uint *)(unaff_ESI + (int)puVar3 * 8);
//     return CONCAT44(iVar4 + iVar5 + (uint)CARRY4(uVar2,uVar6) +
//                     *(int *)(unaff_ESI + 4 + (int)puVar3 * 8) + (uint)CARRY4(uVar2 + uVar6,uVar1),
//                     uVar2 + uVar6 + uVar1);
//   }
//   return CONCAT44(iVar4 + iVar5 + (uint)CARRY4(uVar2,uVar6),uVar2 + uVar6);
// }
// ABI / bounds, original assembly:
// 00476290 SUB ESP,0x8
// 00476293 PUSH EBX
// 00476294 PUSH EBP
// 00476295 XOR EAX,EAX
// 00476297 PUSH EDI
// 00476298 XOR EDI,EDI
// 0047629a XOR EBX,EBX
// 004762fb POP EDI
// 004762fc ADD EAX,EBP
// 004762fe POP EBP
// 004762ff ADC EDX,ECX
// 00476301 POP EBX
// 00476302 ADD ESP,0x8
// 00476305 RET
// Incoming reference SzDecode2:
// 00476731 CMP ECX,0x303011b
// 00476737 JNZ 0x0047694f
// 0047673d CMP EAX,EBP
// 0047673f JNZ 0x0047694f
// 00476745 MOV ESI,dword ptr [ESP + 0x44]
// 00476749 PUSH 0x1
// 0047674b CALL 0x00476290
// 00476750 MOV EDI,dword ptr [ESI + 0x8]
// 00476753 MOV ESI,dword ptr [ESI + 0xc]
// 00476756 ADD ESP,0x4
// VC9 /GL /LTCG asmcmp 2026-10-22: 96.08%.
// FUNCTION: VC1 0x476290
// GetSum

// Ghidra VirtualCity.Unwrapped.exe 0x476310..004764bd.
// Upstream 7zDecode.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int CheckSupportedFolder(_CFolder *param0)
//
// {
//   uint uVar1;
//   int *piVar2;
//   uint *unaff_EDI;
//   bool bVar3;
//
//   uVar1 = *unaff_EDI;
//   if ((uVar1 == 0) || (4 < uVar1)) {
//     return -0x7fffbfff;
//   }
//   piVar2 = (int *)unaff_EDI[1];
//   if (piVar2[2] != 0 || piVar2[3] != 0) {
//     if (piVar2[2] != 0x30101) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[3] != 0) {
//       return -0x7fffbfff;
//     }
//   }
//   if (*piVar2 != 1) {
//     return -0x7fffbfff;
//   }
//   if (piVar2[1] != 1) {
//     return -0x7fffbfff;
//   }
//   if (uVar1 == 1) {
//     if (unaff_EDI[4] != 1) {
//       return -0x7fffbfff;
//     }
//     if (*(int *)unaff_EDI[5] != 0) {
//       return -0x7fffbfff;
//     }
//     bVar3 = unaff_EDI[2] == 0;
//   }
//   else if (uVar1 == 2) {
//     if (piVar2[8] != 0x3030103) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[9] != 0) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[6] != 1) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[7] != 1) {
//       return -0x7fffbfff;
//     }
//     if (unaff_EDI[4] != 1) {
//       return -0x7fffbfff;
//     }
//     if (*(int *)unaff_EDI[5] != 0) {
//       return -0x7fffbfff;
//     }
//     if (unaff_EDI[2] != 1) {
//       return -0x7fffbfff;
//     }
//     if (*(int *)unaff_EDI[3] != 1) {
//       return -0x7fffbfff;
//     }
//     bVar3 = ((int *)unaff_EDI[3])[1] == 0;
//   }
//   else {
//     if (uVar1 != 4) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[8] != 0 || piVar2[9] != 0) {
//       if (piVar2[8] != 0x30101) {
//         return -0x7fffbfff;
//       }
//       if (piVar2[9] != 0) {
//         return -0x7fffbfff;
//       }
//     }
//     if (piVar2[6] != 1) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[7] != 1) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[0xe] != 0 || piVar2[0xf] != 0) {
//       if (piVar2[0xe] != 0x30101) {
//         return -0x7fffbfff;
//       }
//       if (piVar2[0xf] != 0) {
//         return -0x7fffbfff;
//       }
//     }
//     if (piVar2[0xc] != 1) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[0xd] != 1) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[0x14] != 0x303011b) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[0x15] != 0) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[0x12] != 4) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[0x13] != 1) {
//       return -0x7fffbfff;
//     }
//     if (unaff_EDI[4] != 4) {
//       return -0x7fffbfff;
//     }
//     piVar2 = (int *)unaff_EDI[5];
//     if (*piVar2 != 2) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[1] != 6) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[2] != 1) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[3] != 0) {
//       return -0x7fffbfff;
//     }
//     if (unaff_EDI[2] != 3) {
//       return -0x7fffbfff;
//     }
//     piVar2 = (int *)unaff_EDI[3];
//     if (*piVar2 != 5) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[1] != 0) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[2] != 4) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[3] != 1) {
//       return -0x7fffbfff;
//     }
//     if (piVar2[4] != 3) {
//       return -0x7fffbfff;
//     }
//     bVar3 = piVar2[5] == 2;
//   }
//   if (!bVar3) {
//     return -0x7fffbfff;
//   }
//   return 0;
// }
// ABI / bounds, original assembly:
// 00476310 PUSH EBX
// 00476311 PUSH ESI
// 00476312 MOV ESI,dword ptr [EDI]
// 00476314 MOV EBX,0x1
// 00476319 CMP ESI,EBX
// 0047631b JC 0x004764b6
// 00476321 CMP ESI,0x4
// 004764af MOV EAX,0x80004001
// 004764b4 POP EBX
// 004764b5 RET
// 004764b6 POP ESI
// 004764b7 MOV EAX,0x80004001
// 004764bc POP EBX
// 004764bd RET
// Incoming reference SzDecode2:
// 0047669c PUSH EDI
// 0047669d MOV EDI,EBX
// 0047669f MOV dword ptr [ESP + 0x28],EBP
// 004766a3 MOV dword ptr [ESP + 0x2c],EBP
// 004766a7 MOV dword ptr [ESP + 0x20],EBP
// 004766ab MOV dword ptr [ESP + 0x1c],EBP
// 004766af CALL 0x00476310
// 004766b4 CMP EAX,EBP
// 004766b6 JNZ 0x0047693a
// 004766bc MOV EDX,dword ptr [EBX]
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x476310
// CheckSupportedFolder

// Ghidra VirtualCity.Unwrapped.exe 0x4764c0..00476518.
// Upstream 7zDecode.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int LzmaReadImp(void *object,uchar **buffer,uint *size)
//
// {
//   uint *puVar1;
//   int *piVar2;
//   void *pvVar3;
//   int iVar4;
//   void *pvVar5;
//
//   puVar1 = size;
//   pvVar3 = object;
//   pvVar5 = (void *)0x100000;
//   if ((*(int *)((int)object + 0xc) == 0) && (*(uint *)((int)object + 8) < 0x100000)) {
//     pvVar5 = *(void **)((int)object + 8);
//   }
//   *size = 0;
//   iVar4 = (*(code *)**(undefined4 **)((int)object + 4))
//                     (*(undefined4 **)((int)object + 4),buffer,pvVar5,&object);
//   *puVar1 = (uint)object;
//   if (object <= pvVar5) {
//     puVar1 = (uint *)((int)pvVar3 + 8);
//     pvVar5 = (void *)*puVar1;
//     *puVar1 = *puVar1 - (int)object;
//     piVar2 = (int *)((int)pvVar3 + 0xc);
//     *piVar2 = *piVar2 - (uint)(pvVar5 < object);
//     return iVar4;
//   }
//   return -0x7fffbffb;
// }
// ABI / bounds, original assembly:
// 004764c0 PUSH ESI
// 004764c1 MOV ESI,dword ptr [ESP + 0x8]
// 004764c5 CMP dword ptr [ESI + 0xc],0x0
// 004764c9 PUSH EDI
// 004764ca MOV EDI,0x100000
// 004764cf JA 0x004764db
// 004764d1 JC 0x004764d8
// 0047650d POP ESI
// 0047650e RET
// 0047650f SUB dword ptr [ESI + 0x8],ECX
// 00476512 POP EDI
// 00476513 SBB dword ptr [ESI + 0xc],0x0
// 00476517 POP ESI
// 00476518 RET
// Incoming reference SzDecodeLzma:
// 004765d4 MOV dword ptr [ESP + 0x10],ECX
// 004765d8 MOV ECX,dword ptr [ESP + 0x3c]
// 004765dc PUSH ESI
// 004765dd PUSH EDI
// 004765de MOV dword ptr [ESP + 0x1c],EDX
// 004765e2 MOV dword ptr [ESP + 0x14],ECX
// 004765e6 MOV dword ptr [ESP + 0x10],0x4764c0
// 004765ee JL 0x00476600
// 004765f0 MOV EAX,dword ptr [EAX + 0x14]
// 004765f3 LEA EDI,[ESP + 0x20]
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x4764c0
// LzmaReadImp

// Ghidra VirtualCity.Unwrapped.exe 0x476520..004765b9.
// Upstream 7zDecode.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzDecodeCopy(ulonglong inSize,_ISzInStream *inStream,uchar *outBuffer)
//
// {
//   ulonglong uVar1;
//   int iVar2;
//   uint uVar3;
//   uchar *param0;
//   int iVar4;
//   uint uVar5;
//   uchar *local_4;
//
//   param0 = outBuffer;
//   uVar1 = inSize;
//   if (inSize != 0) {
//     do {
//       iVar4 = (int)(uVar1 >> 0x20);
//       uVar3 = (uint)uVar1;
//       uVar5 = 0x40000;
//       if ((iVar4 == 0) && (uVar3 < 0x40000)) {
//         uVar5 = uVar3;
//       }
//       iVar2 = (*inStream->Read)(inStream,&local_4,uVar5,&inSize);
//       if (iVar2 != 0) {
//         return iVar2;
//       }
//       if ((uint)inSize == 0) {
//         return 1;
//       }
//       if (uVar5 < (uint)inSize) {
//         return -0x7fffbffb;
//       }
//       memcpy(param0,local_4,(uint)inSize);
//       param0 = param0 + (uint)inSize;
//       iVar2 = uVar3 - (uint)inSize;
//       iVar4 = iVar4 - (uint)(uVar3 < (uint)inSize);
//       uVar1 = CONCAT44(iVar4,iVar2);
//     } while ((iVar4 != 0) || (uVar1 = CONCAT44(iVar4,iVar2), iVar2 != 0));
//   }
//   return 0;
// }
// ABI / bounds, original assembly:
// 00476520 PUSH ECX
// 00476521 PUSH EBX
// 00476522 MOV EBX,dword ptr [ESP + 0xc]
// 00476526 PUSH EBP
// 00476527 MOV EBP,dword ptr [ESP + 0x1c]
// 0047652b PUSH ESI
// 0047652c MOV ESI,dword ptr [ESP + 0x18]
// 004765af POP EDI
// 004765b0 POP ESI
// 004765b1 POP EBP
// 004765b2 MOV EAX,0x80004005
// 004765b7 POP EBX
// 004765b8 POP ECX
// 004765b9 RET
// Incoming reference SzDecode2:
// 004767aa TEST EDI,EDI
// 004767ac JNZ 0x00476942
// 004767b2 PUSH EAX
// 004767b3 PUSH EBP
// 004767b4 PUSH ESI
// 004767b5 PUSH EDI
// 004767b6 CALL 0x00476520
// 004767bb ADD ESP,0x10
// 004767be TEST EAX,EAX
// 004767c0 JNZ 0x0047693a
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x476520
// SzDecodeCopy

// Ghidra VirtualCity.Unwrapped.exe 0x4765c0..0047668b.
// Upstream 7zDecode.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzDecodeLzma(_CCoderInfo *param0,ulonglong param1,_ISzInStream *param2,uchar *param3,uint param4
//                 ,_ISzAlloc *param5)
//
// {
//   _ISzInStream *outSize;
//   int in_EAX;
//   int iVar1;
//   undefined4 *unaff_EBX;
//   int unaff_EBP;
//   uchar *unaff_ESI;
//   _CLzmaProperties *unaff_EDI;
//   _ILzmaInCallback local_28;
//   undefined4 local_24;
//   _CCoderInfo *local_20;
//   _CLzmaDecoderState local_18;
//
//   outSize = param2;
//   local_20 = param0;
//   local_24 = (_ISzInStream *)param1;
//   local_28.Read = LzmaReadImp;
//   if (4 < *(int *)(in_EAX + 0x10)) {
//     iVar1 = LzmaDecodeProperties(unaff_EDI,unaff_ESI,unaff_EBP);
//     if (iVar1 == 0) {
//       local_18.Probs =
//            (uint *)(*(code *)*unaff_EBX)
//                              ((0x300 << ((char)local_18.Properties.lp + (char)local_18.Properties.lc
//                                         & 0x1fU)) * 4 + 0x1cd8);
//       if (local_18.Probs == (uint *)0x0) {
//         return -0x7ff8fff2;
//       }
//       iVar1 = LzmaDecode(&local_18,&local_28,param1._4_4_,(uint)outSize,(uint *)&param1);
//       (*(code *)unaff_EBX[1])(local_18.Probs);
//       if (iVar1 == 1) {
//         return 1;
//       }
//       if (iVar1 == 0) {
//         return (uint)((_ISzInStream *)param1 != outSize);
//       }
//     }
//   }
//   return -0x7fffbffb;
// }
// ABI / bounds, original assembly:
// 004765c0 SUB ESP,0x2c
// 004765c3 CMP dword ptr [EAX + 0x10],0x5
// 004765c7 MOV ECX,dword ptr [ESP + 0x30]
// 004765cb MOV EDX,dword ptr [ESP + 0x34]
// 004765cf PUSH EBP
// 004765d0 MOV EBP,dword ptr [ESP + 0x44]
// 004765d4 MOV dword ptr [ESP + 0x10],ECX
// 0047667e CMP dword ptr [ESP + 0x44],EBP
// 00476682 POP EDI
// 00476683 POP ESI
// 00476684 SETNZ AL
// 00476687 POP EBP
// 00476688 ADD ESP,0x2c
// 0047668b RET
// Incoming reference SzDecode2:
// 0047690a PUSH EDX
// 0047690b PUSH EDI
// 0047690c PUSH EBP
// 0047690d PUSH EBX
// 0047690e MOV EBX,dword ptr [ESP + 0x70]
// 00476912 PUSH ESI
// 00476913 CALL 0x004765c0
// 00476918 ADD ESP,0x14
// 0047691b TEST EAX,EAX
// 0047691d JNZ 0x0047693a
// VC9 /GL /LTCG asmcmp 2026-10-22: 53.85%.
// FUNCTION: VC1 0x4765c0
// SzDecodeLzma

// Ghidra VirtualCity.Unwrapped.exe 0x476690..00476968.
// Upstream 7zDecode.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzDecode2(ulonglong *packSizes,_CFolder *folder,_ISzInStream *inStream,ulonglong param3,
//              uchar *param4,uint param5,_ISzAlloc *param6,uchar **param7)
//
// {
//   _CCoderInfo *p_Var1;
//   int iVar2;
//   int iVar3;
//   uchar *outBuffer;
//   uchar *puVar4;
//   uint uVar5;
//   _ISzAlloc *unaff_EBP;
//   ulonglong *param0;
//   uint *unaff_ESI;
//   _CFolder *unaff_EDI;
//   ulonglong uVar6;
//   ulonglong uVar7;
//   undefined4 in_stack_00000010;
//   uint local_2c;
//   _CCoderInfo *local_28;
//   uchar *local_24;
//   _CCoderInfo *local_20;
//   _CCoderInfo *local_1c;
//   uint local_18;
//   uint local_14 [5];
//
//   local_18 = 0;
//   local_14[0] = 0;
//   local_20 = (_CCoderInfo *)0x0;
//   local_24 = (uchar *)0x0;
//   iVar2 = CheckSupportedFolder(unaff_EDI);
//   if (iVar2 == 0) {
//     uVar5 = folder->NumCoders;
//     local_2c = 0;
//     if (uVar5 != 0) {
//       do {
//         param0 = (ulonglong *)0x0;
//         local_1c = folder->Coders + local_2c;
//         iVar2 = (int)local_1c->MethodID;
//         iVar3 = *(int *)((int)&local_1c->MethodID + 4);
//         if ((iVar2 == 0 && iVar3 == 0) || ((iVar2 == 0x30101 && (iVar3 == 0)))) {
//           local_28 = (_CCoderInfo *)param4;
//           puVar4 = param3._4_4_;
//           if (uVar5 == 4) {
//             p_Var1 = *(_CCoderInfo **)(folder->UnPackSizes + local_2c);
//             iVar2 = *(int *)((int)folder->UnPackSizes + local_2c * 8 + 4);
//             local_14[2] = 3;
//             local_14[3] = 2;
//             local_14[4] = 0;
//             param0 = (ulonglong *)local_14[local_2c + 2];
//             local_28 = p_Var1;
//             if (local_2c < 2) {
//               if (iVar2 != 0) {
//                 return -0x7ff8fff2;
//               }
//               puVar4 = (uchar *)(**(code **)param5)(p_Var1);
//               if ((puVar4 == (uchar *)0x0) && (p_Var1 != (_CCoderInfo *)0x0)) {
//                 return -0x7ff8fff2;
//               }
//               *(uchar **)(local_2c * -4 + 4 + (int)param6) = puVar4;
//               local_14[-local_2c] = (uint)p_Var1;
//             }
//             else {
//               if (local_2c != 2) {
//                 return -0x7fffbfff;
//               }
//               if (iVar2 != 0) {
//                 return -0x7ff8fff2;
//               }
//               if (param4 < p_Var1) {
//                 return -0x7ff8fff2;
//               }
//               puVar4 = param4 + ((int)param3._4_4_ - (int)p_Var1);
//               local_24 = puVar4;
//               local_20 = p_Var1;
//             }
//           }
//           uVar7 = GetSum(param0,(uint)unaff_EDI);
//           p_Var1 = *(_CCoderInfo **)(packSizes + (int)param0);
//           iVar2 = *(int *)((int)packSizes + (int)param0 * 8 + 4);
//           iVar3 = (*inStream->Seek)(inStream,uVar7 + CONCAT44((undefined4)param3,in_stack_00000010))
//           ;
//           if (iVar3 != 0) {
//             return iVar3;
//           }
//           if ((int)local_1c->MethodID == 0 && *(int *)((int)&local_1c->MethodID + 4) == 0) {
//             if ((p_Var1 != local_28) || (iVar2 != 0)) {
//               return 1;
//             }
//             iVar2 = SzDecodeCopy(ZEXT48(p_Var1),inStream,puVar4);
//           }
//           else {
//             iVar2 = SzDecodeLzma(p_Var1,CONCAT44(puVar4,inStream),(_ISzInStream *)local_28,
//                                  (uchar *)unaff_EDI,(uint)unaff_ESI,unaff_EBP);
//           }
// LAB_0047691b:
//           if (iVar2 != 0) {
//             return iVar2;
//           }
//         }
//         else {
//           if ((iVar2 != 0x3030103) || (iVar3 != 0)) {
//             if ((iVar2 != 0x303011b) || (iVar3 != 0)) {
//               return -0x7fffbfff;
//             }
//             uVar6 = GetSum((ulonglong *)&DAT_00000001,(uint)unaff_EDI);
//             puVar4 = *(uchar **)(packSizes + 1);
//             iVar2 = *(int *)((int)packSizes + 0xc);
//             uVar7 = packSizes[1];
//             if (local_2c != 3) {
//               return -0x7fffbfff;
//             }
//             iVar3 = (*inStream->Seek)(inStream,uVar6 + CONCAT44((undefined4)param3,in_stack_00000010
//                                                                ));
//             if (iVar3 != 0) {
//               return iVar3;
//             }
//             if (iVar2 != 0) {
//               return -0x7ff8fff2;
//             }
//             outBuffer = (uchar *)(**(code **)param5)(puVar4);
//             param6[1].Alloc = outBuffer;
//             if ((outBuffer == (uchar *)0x0) && (puVar4 != (uchar *)0x0)) {
//               return -0x7ff8fff2;
//             }
//             iVar2 = SzDecodeCopy(uVar7,inStream,outBuffer);
//             if (iVar2 != 0) {
//               return iVar2;
//             }
//             iVar2 = x86_2_Decode(local_24,(uint)local_20,param6->Alloc,local_18,param6->Free,
//                                  local_14[0],puVar4,(uint)param3._4_4_,param4,(uint)unaff_EDI);
//             goto LAB_0047691b;
//           }
//           if (local_2c != 1) {
//             return -0x7fffbfff;
//           }
//           local_28 = (_CCoderInfo *)0x0;
//           x86_Convert(param3._4_4_,(uint)&local_28,(uint)unaff_EDI,unaff_ESI,(int)unaff_EBP);
//         }
//         uVar5 = folder->NumCoders;
//         local_2c = local_2c + 1;
//       } while (local_2c < uVar5);
//     }
//     iVar2 = 0;
//   }
//   return iVar2;
// }
// ABI / bounds, original assembly:
// 00476690 SUB ESP,0x30
// 00476693 PUSH EBX
// 00476694 MOV EBX,dword ptr [ESP + 0x3c]
// 00476698 PUSH EBP
// 00476699 XOR EBP,EBP
// 0047669b PUSH ESI
// 0047669c PUSH EDI
// 0047695c POP EDI
// 0047695d POP ESI
// 0047695e POP EBP
// 0047695f MOV EAX,0x1
// 00476964 POP EBX
// 00476965 ADD ESP,0x30
// 00476968 RET
// Incoming reference SzDecode:
// 00476994 XOR ESI,ESI
// 00476996 PUSH EAX
// 00476997 PUSH ECX
// 00476998 MOV dword ptr [ESP + 0x30],ESI
// 0047699c MOV dword ptr [ESP + 0x34],ESI
// 004769a0 MOV dword ptr [ESP + 0x38],ESI
// 004769a4 CALL 0x00476690
// 004769a9 ADD ESP,0x24
// 004769ac MOV EBX,EAX
// 004769ae MOV EDI,EDI
// VC9 /GL /LTCG asmcmp 2026-10-22: 64.36%.
// FUNCTION: VC1 0x476690
// SzDecode2

// Ghidra VirtualCity.Unwrapped.exe 0x476970..004769ca.
// Upstream 7zDecode.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzDecode(ulonglong *packSizes,_CFolder *folder,_ISzInStream *inStream,ulonglong param3,
//             uchar *param4,uint param5,_ISzAlloc *param6)
//
// {
//   int iVar1;
//   uchar *in_ECX;
//   undefined4 in_EDX;
//   uchar **unaff_ESI;
//   int iVar2;
//   uint unaff_EDI;
//   _ISzAlloc local_c;
//   undefined4 local_4;
//
//   iVar2 = 0;
//   local_c.Alloc = (void *)0x0;
//   local_c.Free = (void *)0x0;
//   local_4 = 0;
//   iVar1 = SzDecode2(packSizes,folder,inStream,CONCAT44(in_EDX,(undefined4)param3),in_ECX,unaff_EDI,
//                     &local_c,unaff_ESI);
//   do {
//     (**(code **)(unaff_EDI + 4))((&local_c.Alloc)[iVar2]);
//     iVar2 = iVar2 + 1;
//   } while (iVar2 < 3);
//   return iVar1;
// }
// ABI / bounds, original assembly:
// 00476970 SUB ESP,0x10
// 00476973 PUSH EBX
// 00476974 PUSH ESI
// 00476975 LEA EAX,[ESP + 0xc]
// 00476979 PUSH EAX
// 0047697a MOV EAX,dword ptr [ESP + 0x30]
// 0047697e PUSH EDI
// 004769be CMP ESI,0x3
// 004769c1 JL 0x004769b0
// 004769c3 POP ESI
// 004769c4 MOV EAX,EBX
// 004769c6 POP EBX
// 004769c7 ADD ESP,0x10
// 004769ca RET
// Incoming reference SzReadAndDecodePackedStreams2:
// 00479991 MOV EDI,EBX
// 00479993 MOV EBX,dword ptr [ESP + 0x38]
// 00479997 PUSH EBP
// 00479998 PUSH EDX
// 00479999 MOV EDX,dword ptr [EBX + 0x4]
// 0047999c MOV ECX,ESI
// 0047999e CALL 0x00476970
// 004799a3 ADD ESP,0x14
// 004799a6 TEST EAX,EAX
// 004799a8 JNZ 0x004799f4
// VC9 /GL /LTCG asmcmp 2026-10-22: 66.67%.
// FUNCTION: VC1 0x476970
// SzDecode

// Ghidra VirtualCity.Unwrapped.exe 0x476a20..0x476a2d.
// Upstream 7zBuffer.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// void SzByteBufferInit(_CSzByteBuffer *param0)
//
// {
//   undefined4 *in_EAX;
//
//   *in_EAX = 0;
//   in_EAX[1] = 0;
//   return;
// }
// ABI / bounds, original assembly:
// 0x476a20 mov dword ptr [eax], 0
// 0x476a26 mov dword ptr [eax + 4], 0
// 0x476a2d ret
// 0x476a20 mov dword ptr [eax], 0
// 0x476a26 mov dword ptr [eax + 4], 0
// 0x476a2d ret
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x476a20
// SzByteBufferInit

// Ghidra VirtualCity.Unwrapped.exe 0x476a30..00476a5c.
// Upstream 7zCrc.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// uint __fastcall CrcUpdate(uint size,void *data,uint param2)
//
// {
//   uint uVar1;
//
//   uVar1 = 0xffffffff;
//   for (; size != 0; size = size - 1) {
//     uVar1 = uVar1 >> 8 ^ *(uint *)(&DAT_004d06a8 + ((*(byte *)data ^ uVar1) & 0xff) * 4);
//     data = (void *)((int)data + 1);
//   }
//   return uVar1;
// }
// ABI / bounds, original assembly:
// 00476a30 OR EAX,0xffffffff
// 00476a33 TEST ECX,ECX
// 00476a35 JBE 0x00476a5c
// 00476a37 PUSH ESI
// 00476a38 JMP 0x00476a40
// 00476a40 MOVZX ESI,byte ptr [EDX]
// 00476a43 XOR ESI,EAX
// 00476a4e XOR EAX,dword ptr [ESI*0x4 + 0x4d06a8]
// 00476a55 DEC ECX
// 00476a56 INC EDX
// 00476a57 TEST ECX,ECX
// 00476a59 JA 0x00476a40
// 00476a5b POP ESI
// 00476a5c RET
// Incoming reference CrcCalc:
// 00476a60 CALL 0x00476a30
// 00476a65 NOT EAX
// 00476a67 RET
// VC9 /GL /LTCG asmcmp 2026-10-22: 94.12%.
// FUNCTION: VC1 0x476a30
// CrcUpdate

// Ghidra VirtualCity.Unwrapped.exe 0x476a60..00476a67.
// Upstream 7zCrc.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// uint __fastcall CrcCalc(void *param0,uint param1)
//
// {
//   uint uVar1;
//   uint unaff_retaddr;
//
//   uVar1 = CrcUpdate((uint)param0,(void *)param1,unaff_retaddr);
//   return ~uVar1;
// }
// ABI / bounds, original assembly:
// 00476a60 CALL 0x00476a30
// 00476a65 NOT EAX
// 00476a67 RET
// 00476a60 CALL 0x00476a30
// 00476a65 NOT EAX
// 00476a67 RET
// Incoming reference SzReadAndDecodePackedStreams2:
// 004799a6 TEST EAX,EAX
// 004799a8 JNZ 0x004799f4
// 004799aa CMP dword ptr [EBP + 0x1c],EAX
// 004799ad JZ 0x004799f2
// 004799af MOV EDX,dword ptr [EBX + 0x4]
// 004799b2 MOV ECX,ESI
// 004799b4 CALL 0x00476a60
// 004799b9 CMP EAX,dword ptr [EBP + 0x20]
// 004799bc JZ 0x004799f2
// 004799be MOV EAX,0x80004005
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x476a60
// CrcCalc

// Ghidra VirtualCity.Unwrapped.exe 0x476a70..0x476a89.
// Upstream 7zItem.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// void SzArchiveDatabaseInit(_CArchiveDatabase *param0)
//
// {
//   undefined4 *in_EAX;
//
//   *in_EAX = 0;
//   in_EAX[1] = 0;
//   in_EAX[2] = 0;
//   in_EAX[3] = 0;
//   in_EAX[4] = 0;
//   in_EAX[5] = 0;
//   in_EAX[6] = 0;
//   in_EAX[7] = 0;
//   return;
// }
// ABI / bounds, original assembly:
// 0x476a70 xor ecx, ecx
// 0x476a72 mov dword ptr [eax], ecx
// 0x476a74 mov dword ptr [eax + 4], ecx
// 0x476a77 mov dword ptr [eax + 8], ecx
// 0x476a7a mov dword ptr [eax + 0xc], ecx
// 0x476a7d mov dword ptr [eax + 0x10], ecx
// 0x476a80 mov dword ptr [eax + 0x14], ecx
// 0x476a77 mov dword ptr [eax + 8], ecx
// 0x476a7a mov dword ptr [eax + 0xc], ecx
// 0x476a7d mov dword ptr [eax + 0x10], ecx
// 0x476a80 mov dword ptr [eax + 0x14], ecx
// 0x476a83 mov dword ptr [eax + 0x18], ecx
// 0x476a86 mov dword ptr [eax + 0x1c], ecx
// 0x476a89 ret
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x476a70
// SzArchiveDatabaseInit

// Ghidra VirtualCity.Unwrapped.exe 0x476a90..0x476aa5.
// Upstream 7zItem.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// void SzFileInit(_CFileItem *param0)
//
// {
//   int in_EAX;
//
//   *(undefined1 *)(in_EAX + 0x18) = 0;
//   *(undefined1 *)(in_EAX + 0x19) = 1;
//   *(undefined1 *)(in_EAX + 0x1a) = 0;
//   *(undefined1 *)(in_EAX + 0x1b) = 0;
//   *(undefined1 *)(in_EAX + 0x1c) = 0;
//   *(undefined4 *)(in_EAX + 0x14) = 0;
//   return;
// }
// ABI / bounds, original assembly:
// 0x476a90 xor ecx, ecx
// 0x476a92 mov byte ptr [eax + 0x18], cl
// 0x476a95 mov byte ptr [eax + 0x19], 1
// 0x476a99 mov byte ptr [eax + 0x1a], cl
// 0x476a9c mov byte ptr [eax + 0x1b], cl
// 0x476a9f mov byte ptr [eax + 0x1c], cl
// 0x476aa2 mov dword ptr [eax + 0x14], ecx
// 0x476a92 mov byte ptr [eax + 0x18], cl
// 0x476a95 mov byte ptr [eax + 0x19], 1
// 0x476a99 mov byte ptr [eax + 0x1a], cl
// 0x476a9c mov byte ptr [eax + 0x1b], cl
// 0x476a9f mov byte ptr [eax + 0x1c], cl
// 0x476aa2 mov dword ptr [eax + 0x14], ecx
// 0x476aa5 ret
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x476a90
// SzFileInit

// Ghidra VirtualCity.Unwrapped.exe 0x476ab0..0x476acf.
// Upstream 7zItem.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzFolderFindBindPairForOutStream(_CFolder *param0,uint param1)
//
// {
//   uint uVar1;
//   int in_ECX;
//   int *piVar2;
//   int unaff_ESI;
//
//   uVar1 = 0;
//   if (*(uint *)(in_ECX + 8) != 0) {
//     piVar2 = (int *)(*(int *)(in_ECX + 0xc) + 4);
//     do {
//       if (*piVar2 == unaff_ESI) {
//         return uVar1;
//       }
//       uVar1 = uVar1 + 1;
//       piVar2 = piVar2 + 2;
//     } while (uVar1 < *(uint *)(in_ECX + 8));
//   }
//   return -1;
// }
// ABI / bounds, original assembly:
// 0x476ab0 mov edx, dword ptr [ecx + 8]
// 0x476ab3 xor eax, eax
// 0x476ab5 test edx, edx
// 0x476ab7 jbe 0x476acc
// 0x476ab9 mov ecx, dword ptr [ecx + 0xc]
// 0x476abc add ecx, 4
// 0x476abf nop
// 0x476ac2 je 0x476acf
// 0x476ac4 inc eax
// 0x476ac5 add ecx, 8
// 0x476ac8 cmp eax, edx
// 0x476aca jb 0x476ac0
// 0x476acc or eax, 0xffffffff
// 0x476acf ret
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x476ab0
// SzFolderFindBindPairForOutStream

// Ghidra VirtualCity.Unwrapped.exe 0x476ad0..0x476aef.
// Upstream 7zItem.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzFolderFindBindPairForInStream(_CFolder *param0,uint param1)
//
// {
//   uint uVar1;
//   int in_EDX;
//   int *piVar2;
//   int unaff_ESI;
//
//   uVar1 = 0;
//   if (*(uint *)(in_EDX + 8) != 0) {
//     piVar2 = *(int **)(in_EDX + 0xc);
//     do {
//       if (*piVar2 == unaff_ESI) {
//         return uVar1;
//       }
//       uVar1 = uVar1 + 1;
//       piVar2 = piVar2 + 2;
//     } while (uVar1 < *(uint *)(in_EDX + 8));
//   }
//   return -1;
// }
// ABI / bounds, original assembly:
// 0x476ad0 mov ecx, dword ptr [edx + 8]
// 0x476ad3 xor eax, eax
// 0x476ad5 test ecx, ecx
// 0x476ad7 jbe 0x476aec
// 0x476ad9 mov edx, dword ptr [edx + 0xc]
// 0x476adc lea esp, [esp]
// 0x476ae0 cmp dword ptr [edx], esi
// 0x476ae2 je 0x476aef
// 0x476ae4 inc eax
// 0x476ae5 add edx, 8
// 0x476ae8 cmp eax, ecx
// 0x476aea jb 0x476ae0
// 0x476aec or eax, 0xffffffff
// 0x476aef ret
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x476ad0
// SzFolderFindBindPairForInStream

// Ghidra VirtualCity.Unwrapped.exe 0x476af0..00476b4a.
// Upstream 7zItem.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// uint SzFolderGetNumOutStreams(_CFolder *folder)
//
// {
//   uint uVar1;
//   _CFolder *p_Var2;
//   uint *puVar3;
//   int iVar4;
//   int iVar5;
//   uint uVar6;
//   int iVar7;
//
//   p_Var2 = folder;
//   uVar1 = folder->NumCoders;
//   uVar6 = 0;
//   iVar5 = 0;
//   iVar7 = 0;
//   folder = (_CFolder *)0x0;
//   if (1 < (int)uVar1) {
//     puVar3 = &p_Var2->Coders[1].NumOutStreams;
//     iVar4 = (uVar1 - 2 >> 1) + 1;
//     uVar6 = iVar4 * 2;
//     do {
//       iVar5 = iVar5 + puVar3[-6];
//       iVar7 = iVar7 + *puVar3;
//       puVar3 = puVar3 + 0xc;
//       iVar4 = iVar4 + -1;
//     } while (iVar4 != 0);
//   }
//   if (uVar6 < uVar1) {
//     folder = (_CFolder *)p_Var2->Coders[uVar6].NumOutStreams;
//   }
//   return (int)&folder->NumCoders + iVar7 + iVar5;
// }
// ABI / bounds, original assembly:
// 00476af0 PUSH EBX
// 00476af1 PUSH EBP
// 00476af2 MOV EBP,dword ptr [ESP + 0xc]
// 00476af6 PUSH ESI
// 00476af7 PUSH EDI
// 00476af8 MOV EDI,dword ptr [EBP]
// 00476afb XOR EBX,EBX
// 00476b3f POP EDI
// 00476b40 LEA EAX,[ESI + EDX*0x1]
// 00476b43 ADD EAX,dword ptr [ESP + 0x10]
// 00476b47 POP ESI
// 00476b48 POP EBP
// 00476b49 POP EBX
// 00476b4a RET
// Incoming reference SzFolderGetUnPackSize:
// 00476bb0 PUSH ESI
// 00476bb1 PUSH EDI
// 00476bb2 CALL 0x00476af0
// 00476bb7 MOV ESI,EAX
// 00476bb9 ADD ESP,0x4
// 00476bbc TEST ESI,ESI
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x476af0
// SzFolderGetNumOutStreams

// Ghidra VirtualCity.Unwrapped.exe 0x476b50..0x476b6f.
// Upstream 7zItem.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// void SzFolderInit(_CFolder *param0)
//
// {
//   undefined4 *in_EAX;
//
//   *in_EAX = 0;
//   in_EAX[1] = 0;
//   in_EAX[2] = 0;
//   in_EAX[3] = 0;
//   in_EAX[4] = 0;
//   in_EAX[5] = 0;
//   in_EAX[6] = 0;
//   in_EAX[7] = 0;
//   in_EAX[8] = 0;
//   in_EAX[9] = 0;
//   return;
// }
// ABI / bounds, original assembly:
// 0x476b50 xor ecx, ecx
// 0x476b52 mov dword ptr [eax], ecx
// 0x476b54 mov dword ptr [eax + 4], ecx
// 0x476b57 mov dword ptr [eax + 8], ecx
// 0x476b5a mov dword ptr [eax + 0xc], ecx
// 0x476b5d mov dword ptr [eax + 0x10], ecx
// 0x476b60 mov dword ptr [eax + 0x14], ecx
// 0x476b5d mov dword ptr [eax + 0x10], ecx
// 0x476b60 mov dword ptr [eax + 0x14], ecx
// 0x476b63 mov dword ptr [eax + 0x18], ecx
// 0x476b66 mov dword ptr [eax + 0x1c], ecx
// 0x476b69 mov dword ptr [eax + 0x20], ecx
// 0x476b6c mov dword ptr [eax + 0x24], ecx
// 0x476b6f ret
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x476b50
// SzFolderInit

// Ghidra VirtualCity.Unwrapped.exe 0x476b70..0x476b78.
// Upstream 7zItem.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// void SzCoderInfoInit(_CCoderInfo *param0)
//
// {
//   int in_EAX;
//
//   *(undefined4 *)(in_EAX + 0x10) = 0;
//   *(undefined4 *)(in_EAX + 0x14) = 0;
//   return;
// }
// ABI / bounds, original assembly:
// 0x476b70 xor ecx, ecx
// 0x476b72 mov dword ptr [eax + 0x10], ecx
// 0x476b75 mov dword ptr [eax + 0x14], ecx
// 0x476b78 ret
// 0x476b70 xor ecx, ecx
// 0x476b72 mov dword ptr [eax + 0x10], ecx
// 0x476b75 mov dword ptr [eax + 0x14], ecx
// 0x476b78 ret
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x476b70
// SzCoderInfoInit

// Ghidra VirtualCity.Unwrapped.exe 0x476b80..0x476ba0.
// Upstream 7zItem.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// void SzFileFree(_CFileItem *param0,void *param1)
//
// {
//   int unaff_ESI;
//
//   (*(code *)param0)(*(undefined4 *)(unaff_ESI + 0x14));
//   *(undefined1 *)(unaff_ESI + 0x18) = 0;
//   *(undefined1 *)(unaff_ESI + 0x19) = 1;
//   *(undefined1 *)(unaff_ESI + 0x1a) = 0;
//   *(undefined1 *)(unaff_ESI + 0x1b) = 0;
//   *(undefined1 *)(unaff_ESI + 0x1c) = 0;
//   *(undefined4 *)(unaff_ESI + 0x14) = 0;
//   return;
// }
// ABI / bounds, original assembly:
// 0x476b80 mov eax, dword ptr [esi + 0x14]
// 0x476b83 push eax
// 0x476b84 call dword ptr [esp + 8]
// 0x476b88 xor eax, eax
// 0x476b8a add esp, 4
// 0x476b8d mov byte ptr [esi + 0x18], al
// 0x476b90 mov byte ptr [esi + 0x19], 1
// 0x476b8d mov byte ptr [esi + 0x18], al
// 0x476b90 mov byte ptr [esi + 0x19], 1
// 0x476b94 mov byte ptr [esi + 0x1a], al
// 0x476b97 mov byte ptr [esi + 0x1b], al
// 0x476b9a mov byte ptr [esi + 0x1c], al
// 0x476b9d mov dword ptr [esi + 0x14], eax
// 0x476ba0 ret
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x476b80
// SzFileFree

// Ghidra VirtualCity.Unwrapped.exe 0x476bb0..00476bfa.
// Upstream 7zItem.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// ulonglong SzFolderGetUnPackSize(_CFolder *param0)
//
// {
//   uint uVar1;
//   uint *puVar2;
//   uint uVar3;
//   _CFolder *unaff_EDI;
//
//   uVar1 = SzFolderGetNumOutStreams(unaff_EDI);
//   if ((uVar1 != 0) && (uVar1 = uVar1 - 1, -1 < (int)uVar1)) {
//     do {
//       uVar3 = 0;
//       if (unaff_EDI->NumBindPairs == 0) {
// LAB_00476be0:
//         return CONCAT44(*(undefined4 *)((int)unaff_EDI->UnPackSizes + uVar1 * 8 + 4),
//                         (int)unaff_EDI->UnPackSizes[uVar1]);
//       }
//       puVar2 = &unaff_EDI->BindPairs->OutIndex;
//       while (*puVar2 != uVar1) {
//         uVar3 = uVar3 + 1;
//         puVar2 = puVar2 + 2;
//         if (unaff_EDI->NumBindPairs <= uVar3) goto LAB_00476be0;
//       }
//       if ((int)uVar3 < 0) goto LAB_00476be0;
//       uVar1 = uVar1 - 1;
//     } while (-1 < (int)uVar1);
//   }
//   return 0;
// }
// ABI / bounds, original assembly:
// 00476bb0 PUSH ESI
// 00476bb1 PUSH EDI
// 00476bb2 CALL 0x00476af0
// 00476bb7 MOV ESI,EAX
// 00476bb9 ADD ESP,0x4
// 00476bbc TEST ESI,ESI
// 00476bbe JZ 0x00476bf5
// 00476bee JL 0x00476be0
// 00476bf0 SUB ESI,0x1
// 00476bf3 JNS 0x00476bc8
// 00476bf5 XOR EAX,EAX
// 00476bf7 XOR EDX,EDX
// 00476bf9 POP ESI
// 00476bfa RET
// Incoming reference SzReadSubStreamsInfo:
// 004790bf MOV dword ptr [ESP + 0x10],ESI
// 004790c3 ADC dword ptr [ESP + 0x30],EDX
// 004790c7 INC EDI
// 004790c8 CMP EDI,EBP
// 004790ca JC 0x00479090
// 004790cc MOV EDI,dword ptr [ESP + 0x14]
// 004790d0 CALL 0x00476bb0
// 004790d5 MOV EDI,dword ptr [ESP + 0x28]
// 004790d9 MOV ESI,EAX
// 004790db SUB ESI,dword ptr [ESP + 0x2c]
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x476bb0
// SzFolderGetUnPackSize

// Ghidra VirtualCity.Unwrapped.exe 0x476c00..0x476c13.
// Upstream 7zItem.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// void SzCoderInfoFree(_CCoderInfo *param0,void *param1)
//
// {
//   int unaff_ESI;
//
//   (*(code *)param0)(*(undefined4 *)(unaff_ESI + 0x14));
//   *(undefined4 *)(unaff_ESI + 0x10) = 0;
//   *(undefined4 *)(unaff_ESI + 0x14) = 0;
//   return;
// }
// ABI / bounds, original assembly:
// 0x476c00 mov eax, dword ptr [esi + 0x14]
// 0x476c03 push eax
// 0x476c04 call dword ptr [esp + 8]
// 0x476c08 xor eax, eax
// 0x476c0a add esp, 4
// 0x476c0d mov dword ptr [esi + 0x10], eax
// 0x476c10 mov dword ptr [esi + 0x14], eax
// 0x476c03 push eax
// 0x476c04 call dword ptr [esp + 8]
// 0x476c08 xor eax, eax
// 0x476c0a add esp, 4
// 0x476c0d mov dword ptr [esi + 0x10], eax
// 0x476c10 mov dword ptr [esi + 0x14], eax
// 0x476c13 ret
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x476c00
// SzCoderInfoFree

// Ghidra VirtualCity.Unwrapped.exe 0x477d80..00477da8.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzSkeepDataSize(_CSzState *param0,ulonglong param1)
//
// {
//   int *in_ECX;
//   int in_stack_00000008;
//
//   if ((in_stack_00000008 == 0) && (param0 <= (_CSzState *)in_ECX[1])) {
//     *in_ECX = (int)&param0->Data + *in_ECX;
//     in_ECX[1] = in_ECX[1] - (int)param0;
//     return 0;
//   }
//   return 6;
// }
// ABI / bounds, original assembly:
// 00477d80 MOV EAX,dword ptr [ECX + 0x4]
// 00477d83 PUSH ESI
// 00477d84 MOV ESI,dword ptr [ESP + 0xc]
// 00477d88 XOR EDX,EDX
// 00477d8a CMP ESI,EDX
// 00477d8c MOV EDX,dword ptr [ESP + 0x8]
// 00477d90 POP ESI
// 00477d99 MOV EAX,0x6
// 00477d9e RET
// 00477d9f ADD dword ptr [ECX],EDX
// 00477da1 SUB EAX,EDX
// 00477da3 MOV dword ptr [ECX + 0x4],EAX
// 00477da6 XOR EAX,EAX
// 00477da8 RET
// Incoming reference SzReadHeader2:
// 00479cee JMP 0x00479d0c
// 00479cf0 MOV EAX,dword ptr [ESP + 0x20]
// 00479cf4 MOV ECX,dword ptr [ESP + 0x1c]
// 00479cf8 PUSH EAX
// 00479cf9 PUSH ECX
// 00479cfa MOV ECX,EBP
// 00479cfc CALL 0x00477d80
// 00479d01 ADD ESP,0x8
// 00479d04 TEST EAX,EAX
// 00479d06 JNZ 0x00479dd6
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x477d80
// SzSkeepDataSize

// Ghidra VirtualCity.Unwrapped.exe 0x477db0..0x477dbc.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadByte(_CSzState *param0,uchar *param1)
//
// {
//   undefined1 uVar1;
//   undefined4 *in_EAX;
//   undefined1 *unaff_ESI;
//
//   if (in_EAX[1] == 0) {
//     return 6;
//   }
//   in_EAX[1] = in_EAX[1] + -1;
//   uVar1 = *(undefined1 *)*in_EAX;
//   *in_EAX = (undefined1 *)*in_EAX + 1;
//   *unaff_ESI = uVar1;
//   return 0;
// }
// ABI / bounds, original assembly:
// 0x477db0 mov ecx, dword ptr [eax + 4]
// 0x477db3 test ecx, ecx
// 0x477db5 jne 0x477dbd
// 0x477db7 mov eax, 6
// 0x477dbc ret
// 0x477db0 mov ecx, dword ptr [eax + 4]
// 0x477db3 test ecx, ecx
// 0x477db5 jne 0x477dbd
// 0x477db7 mov eax, 6
// 0x477dbc ret
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x477db0
// SzReadByte

// Ghidra VirtualCity.Unwrapped.exe 0x477dd0..00477dfd.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int TestSignatureCandidate(uchar *param0)
//
// {
//   uint uVar1;
//   int in_ECX;
//
//   uVar1 = 0;
//   do {
//     if (*(char *)(in_ECX + uVar1) != (&DAT_004e89a0)[uVar1]) {
//       return 0;
//     }
//     uVar1 = uVar1 + 1;
//   } while (uVar1 < 6);
//   return 1;
// }
// ABI / bounds, original assembly:
// 00477dd0 XOR EAX,EAX
// 00477dd2 SUB ECX,0x4e89a0
// 00477dd8 JMP 0x00477de0
// 00477de0 MOV DL,byte ptr [ECX + EAX*0x1 + 0x4e89a0]
// 00477de7 CMP DL,byte ptr [EAX + 0x4e89a0]
// 00477ded JNZ 0x00477dfb
// 00477def INC EAX
// 00477def INC EAX
// 00477df0 CMP EAX,0x6
// 00477df3 JC 0x00477de0
// 00477df5 MOV EAX,0x1
// 00477dfa RET
// 00477dfb XOR EAX,EAX
// 00477dfd RET
// Incoming reference SzArchiveOpen2:
// 00479f79 MOV dword ptr [ESP + 0x14],0x0
// 00479f81 CALL 0x00477e00
// 00479f86 ADD ESP,0x4
// 00479f89 TEST EAX,EAX
// 00479f8b JNZ 0x0047a1f1
// 00479f91 LEA ECX,[ESP + 0x20]
// 00479f95 CALL 0x00477dd0
// 00479f9a TEST EAX,EAX
// 00479f9c JNZ 0x00479faa
// 00479f9e MOV EAX,0x6
// VC9 /GL /LTCG asmcmp 2026-10-22: 78.57%.
// FUNCTION: VC1 0x477dd0
// TestSignatureCandidate

// Ghidra VirtualCity.Unwrapped.exe 0x477e00..00477e67.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SafeReadDirect(_ISzInStream *inStream,uchar *param1,uint param2)
//
// {
//   uint in_EAX;
//   int iVar1;
//   undefined1 *puVar2;
//   undefined1 *in_ECX;
//   uint local_8;
//   undefined1 *local_4;
//
//   while( true ) {
//     if (in_EAX == 0) {
//       return 0;
//     }
//     iVar1 = (*inStream->Read)(inStream,&local_4,in_EAX,&local_8);
//     if (iVar1 != 0) break;
//     if ((local_8 == 0) || (in_EAX < local_8)) {
//       return -0x7fffbffb;
//     }
//     in_EAX = in_EAX - local_8;
//     puVar2 = local_4;
//     do {
//       *in_ECX = *puVar2;
//       in_ECX = in_ECX + 1;
//       puVar2 = puVar2 + 1;
//       local_8 = local_8 - 1;
//     } while (local_8 != 0);
//   }
//   return iVar1;
// }
// ABI / bounds, original assembly:
// 00477e00 SUB ESP,0x8
// 00477e03 PUSH EBP
// 00477e04 MOV EBP,dword ptr [ESP + 0x10]
// 00477e08 PUSH ESI
// 00477e09 PUSH EDI
// 00477e0a MOV EDI,EAX
// 00477e0c MOV ESI,ECX
// 00477e5b RET
// 00477e5c POP EDI
// 00477e5d POP ESI
// 00477e5e MOV EAX,0x80004005
// 00477e63 POP EBP
// 00477e64 ADD ESP,0x8
// 00477e67 RET
// Incoming reference SzArchiveOpen2:
// 00479f6b MOV ESI,dword ptr [EBP + 0x8]
// 00479f6e PUSH EDI
// 00479f6f PUSH ESI
// 00479f70 MOV EAX,0x6
// 00479f75 LEA ECX,[ESP + 0x24]
// 00479f79 MOV dword ptr [ESP + 0x14],0x0
// 00479f81 CALL 0x00477e00
// 00479f86 ADD ESP,0x4
// 00479f89 TEST EAX,EAX
// 00479f8b JNZ 0x0047a1f1
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x477e00
// SafeReadDirect

// Ghidra VirtualCity.Unwrapped.exe 0x477e70..0x477e8c.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// ulonglong SzArDbGetFolderStreamPos(_CArchiveDatabaseEx *param0,uint param1,uint param2)
//
// {
//   int iVar1;
//   uint uVar2;
//   int in_ECX;
//
//   iVar1 = *(int *)(*(int *)(in_ECX + 0x30) + (int)param0 * 4);
//   uVar2 = *(uint *)(*(int *)(in_ECX + 0x34) + iVar1 * 8);
//   return CONCAT44(*(int *)(*(int *)(in_ECX + 0x34) + 4 + iVar1 * 8) + *(int *)(in_ECX + 0x2c) +
//                   (uint)CARRY4(uVar2,*(uint *)(in_ECX + 0x28)),uVar2 + *(uint *)(in_ECX + 0x28));
// }
// ABI / bounds, original assembly:
// 0x477e70 mov eax, dword ptr [ecx + 0x30]
// 0x477e73 mov edx, dword ptr [ecx + 0x34]
// 0x477e76 push esi
// 0x477e77 mov esi, dword ptr [esp + 8]
// 0x477e7b mov esi, dword ptr [eax + esi*4]
// 0x477e7e mov eax, dword ptr [edx + esi*8]
// 0x477e81 add eax, dword ptr [ecx + 0x28]
// 0x477e7b mov esi, dword ptr [eax + esi*4]
// 0x477e7e mov eax, dword ptr [edx + esi*8]
// 0x477e81 add eax, dword ptr [ecx + 0x28]
// 0x477e84 mov edx, dword ptr [edx + esi*8 + 4]
// 0x477e88 adc edx, dword ptr [ecx + 0x2c]
// 0x477e8b pop esi
// 0x477e8c ret
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x477e70
// SzArDbGetFolderStreamPos

// Ghidra VirtualCity.Unwrapped.exe 0x477e90..00478039.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzArDbExFill(_CArchiveDatabaseEx *param0,void *param1)
//
// {
//   uint uVar1;
//   uint uVar2;
//   uint uVar3;
//   int iVar4;
//   uint uVar5;
//   int iVar6;
//   uint *unaff_ESI;
//   bool bVar7;
//   uint local_c;
//   int local_8;
//
//   iVar6 = 0;
//   uVar5 = 0;
//   local_c = 0;
//   if (unaff_ESI[4] == 0) {
//     unaff_ESI[0xc] = 0;
//   }
//   else {
//     uVar2 = (*(code *)param0)(unaff_ESI[4] * 4);
//     unaff_ESI[0xc] = uVar2;
//     if (uVar2 == 0) {
//       return -0x7ff8fff2;
//     }
//   }
//   uVar2 = 0;
//   if (unaff_ESI[4] != 0) {
//     iVar4 = 0;
//     do {
//       *(int *)(unaff_ESI[0xc] + uVar2 * 4) = iVar6;
//       iVar6 = iVar6 + *(int *)(unaff_ESI[5] + 0x10 + iVar4);
//       uVar2 = uVar2 + 1;
//       iVar4 = iVar4 + 0x28;
//     } while (uVar2 < unaff_ESI[4]);
//   }
//   uVar2 = 0;
//   if (*unaff_ESI == 0) {
//     unaff_ESI[0xd] = 0;
//   }
//   else {
//     uVar3 = (*(code *)param0)(*unaff_ESI * 8);
//     unaff_ESI[0xd] = uVar3;
//     if (uVar3 == 0) {
//       return -0x7ff8fff2;
//     }
//   }
//   iVar6 = 0;
//   uVar3 = 0;
//   if (*unaff_ESI != 0) {
//     do {
//       uVar1 = unaff_ESI[0xd];
//       *(uint *)(uVar1 + uVar3 * 8) = uVar5;
//       *(int *)(uVar1 + 4 + uVar3 * 8) = iVar6;
//       uVar1 = *(uint *)(unaff_ESI[1] + uVar3 * 8);
//       bVar7 = CARRY4(uVar5,uVar1);
//       uVar5 = uVar5 + uVar1;
//       iVar6 = iVar6 + *(int *)(unaff_ESI[1] + 4 + uVar3 * 8) + (uint)bVar7;
//       uVar3 = uVar3 + 1;
//     } while (uVar3 < *unaff_ESI);
//   }
//   if (unaff_ESI[4] == 0) {
//     unaff_ESI[0xe] = 0;
//   }
//   else {
//     uVar5 = (*(code *)param0)(unaff_ESI[4] * 4);
//     unaff_ESI[0xe] = uVar5;
//     if (uVar5 == 0) {
//       return -0x7ff8fff2;
//     }
//   }
//   if (unaff_ESI[6] == 0) {
//     unaff_ESI[0xf] = 0;
//   }
//   else {
//     uVar5 = (*(code *)param0)(unaff_ESI[6] * 4);
//     unaff_ESI[0xf] = uVar5;
//     if (uVar5 == 0) {
//       return -0x7ff8fff2;
//     }
//   }
//   uVar5 = 0;
//   if (unaff_ESI[6] != 0) {
//     local_8 = 0;
//     do {
//       bVar7 = *(char *)(unaff_ESI[7] + 0x19 + local_8) != '\0';
//       if (bVar7) {
//         if (local_c == 0) {
//           iVar6 = uVar2 * 0x28;
//           while( true ) {
//             if (unaff_ESI[4] <= uVar2) {
//               return 6;
//             }
//             *(uint *)(unaff_ESI[0xe] + uVar2 * 4) = uVar5;
//             if (*(int *)(iVar6 + 0x24 + unaff_ESI[5]) != 0) break;
//             uVar2 = uVar2 + 1;
//             iVar6 = iVar6 + 0x28;
//           }
//         }
// LAB_00477ff2:
//         *(uint *)(unaff_ESI[0xf] + uVar5 * 4) = uVar2;
//         if ((bVar7) &&
//            (local_c = local_c + 1, *(uint *)(unaff_ESI[5] + 0x24 + uVar2 * 0x28) <= local_c)) {
//           uVar2 = uVar2 + 1;
//           local_c = 0;
//         }
//       }
//       else {
//         if (local_c != 0) goto LAB_00477ff2;
//         *(undefined4 *)(unaff_ESI[0xf] + uVar5 * 4) = 0xffffffff;
//       }
//       local_8 = local_8 + 0x20;
//       uVar5 = uVar5 + 1;
//     } while (uVar5 < unaff_ESI[6]);
//   }
//   return 0;
// }
// ABI / bounds, original assembly:
// 00477e90 SUB ESP,0xc
// 00477e93 MOV EAX,dword ptr [ESI + 0x10]
// 00477e96 PUSH EBX
// 00477e97 PUSH EBP
// 00477e98 XOR EDX,EDX
// 00477e9a PUSH EDI
// 00477e9b XOR EBP,EBP
// 0047802d RET
// 0047802e POP EDI
// 0047802f POP EBP
// 00478030 MOV EAX,0x6
// 00478035 POP EBX
// 00478036 ADD ESP,0xc
// 00478039 RET
// Incoming reference SzReadHeader2:
// 00479dbd CMP ESI,dword ptr [ESP + 0x10]
// 00479dc1 JC 0x00479d46
// 00479dc3 MOV EAX,dword ptr [ESP + 0x50]
// 00479dc7 MOV ECX,dword ptr [EAX]
// 00479dc9 MOV ESI,dword ptr [ESP + 0x34]
// 00479dcd PUSH ECX
// 00479dce CALL 0x00477e90
// 00479dd3 ADD ESP,0x4
// 00479dd6 POP EDI
// 00479dd7 POP ESI
// VC9 /GL /LTCG asmcmp 2026-10-22: 98.77%.
// FUNCTION: VC1 0x477e90
// SzArDbExFill

// Ghidra VirtualCity.Unwrapped.exe 0x478040..0x478065.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// void SzArDbExInit(_CArchiveDatabaseEx *param0)
//
// {
//   undefined4 *in_EAX;
//
//   *in_EAX = 0;
//   in_EAX[1] = 0;
//   in_EAX[2] = 0;
//   in_EAX[3] = 0;
//   in_EAX[4] = 0;
//   in_EAX[5] = 0;
//   in_EAX[6] = 0;
//   in_EAX[7] = 0;
//   in_EAX[0xc] = 0;
//   in_EAX[0xd] = 0;
//   in_EAX[0xe] = 0;
//   in_EAX[0xf] = 0;
//   return;
// }
// ABI / bounds, original assembly:
// 0x478040 xor ecx, ecx
// 0x478042 mov dword ptr [eax], ecx
// 0x478044 mov dword ptr [eax + 4], ecx
// 0x478047 mov dword ptr [eax + 8], ecx
// 0x47804a mov dword ptr [eax + 0xc], ecx
// 0x47804d mov dword ptr [eax + 0x10], ecx
// 0x478050 mov dword ptr [eax + 0x14], ecx
// 0x478053 mov dword ptr [eax + 0x18], ecx
// 0x478056 mov dword ptr [eax + 0x1c], ecx
// 0x478059 mov dword ptr [eax + 0x30], ecx
// 0x47805c mov dword ptr [eax + 0x34], ecx
// 0x47805f mov dword ptr [eax + 0x38], ecx
// 0x478062 mov dword ptr [eax + 0x3c], ecx
// 0x478065 ret
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x478040
// SzArDbExInit

// Ghidra VirtualCity.Unwrapped.exe 0x478070..004782b7.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadFileNames(_CSzState *sd,uint numFiles,_CFileItem *param2,void *param3)
//
// {
//   ushort uVar1;
//   int in_EAX;
//   uint uVar2;
//   uint uVar3;
//   int iVar4;
//   int iVar5;
//   uchar *puVar6;
//   uchar *puVar7;
//   uint uVar8;
//   uint uVar9;
//   uchar *puVar10;
//   uchar *puVar11;
//   int iVar12;
//   int iVar13;
//   int local_10;
//   uint local_c;
//
//   local_c = 0;
//   local_10 = in_EAX;
//   if (numFiles != 0) {
//     do {
//       if (sd->Size < 2) {
// LAB_0047815b:
//         *(undefined4 *)(local_10 + 0x14) = 0;
//       }
//       else {
//         iVar4 = 0;
//         puVar6 = sd->Data;
//         uVar8 = 2;
//         puVar10 = sd->Data + 1;
//         do {
//           uVar2 = (uint)CONCAT11(*puVar10,*puVar6);
//           iVar4 = iVar4 + 1;
//           puVar11 = puVar10 + 2;
//           puVar7 = puVar6 + 2;
//           uVar9 = uVar8 + 2;
//           if (uVar2 == 0) break;
//           if (0x7f < uVar2) {
//             if (uVar2 - 0xd800 < 0x800) {
//               if ((0xdbff < uVar2) || (sd->Size < uVar9)) {
//                 return 6;
//               }
//               uVar3 = CONCAT11(*puVar11,*puVar7) - 0xdc00;
//               puVar11 = puVar10 + 4;
//               puVar7 = puVar6 + 4;
//               uVar9 = uVar8 + 4;
//               if (0x3ff < uVar3) {
//                 return 6;
//               }
//               uVar2 = (uVar2 - 0xd800) * 0x400 | uVar3;
//             }
//             iVar12 = 1;
//             iVar5 = 0xb;
//             do {
//               if (uVar2 < (uint)(1 << ((byte)iVar5 & 0x1f))) break;
//               iVar5 = iVar5 + 5;
//               iVar12 = iVar12 + 1;
//             } while (iVar5 < 0x1f);
//             iVar4 = iVar4 + iVar12;
//           }
//           puVar6 = puVar7;
//           uVar8 = uVar9;
//           puVar10 = puVar11;
//         } while (uVar9 <= sd->Size);
//         if (iVar4 == 0) goto LAB_0047815b;
//         iVar4 = (*(code *)param2)(iVar4);
//         *(int *)(local_10 + 0x14) = iVar4;
//         if (iVar4 == 0) {
//           return -0x7ff8fff2;
//         }
//       }
//       iVar4 = 0;
//       uVar8 = sd->Size;
//       while (1 < uVar8) {
//         uVar1 = *(ushort *)sd->Data;
//         uVar8 = (uint)uVar1;
//         if (1 < sd->Size) {
//           sd->Size = sd->Size - 2;
//           sd->Data = (uchar *)((int)sd->Data + 2);
//         }
//         if (uVar8 < 0x80) {
//           *(char *)(iVar4 + *(int *)(local_10 + 0x14)) = (char)uVar1;
//           iVar4 = iVar4 + 1;
//           if (uVar8 == 0) break;
//         }
//         else {
//           if (uVar8 - 0xd800 < 0x800) {
//             uVar1 = *(ushort *)sd->Data;
//             if (1 < sd->Size) {
//               sd->Size = sd->Size - 2;
//               sd->Data = (uchar *)((int)sd->Data + 2);
//             }
//             uVar8 = uVar1 - 0xdc00 | (uVar8 - 0xd800) * 0x400;
//           }
//           iVar5 = 1;
//           iVar12 = 0xb;
//           do {
//             if (uVar8 < (uint)(1 << ((byte)iVar12 & 0x1f))) break;
//             iVar12 = iVar12 + 5;
//             iVar5 = iVar5 + 1;
//           } while (iVar12 < 0x1f);
//           iVar12 = iVar5 * 6;
//           iVar13 = iVar4 + 1;
//           *(char *)(iVar4 + *(int *)(local_10 + 0x14)) =
//                (char)(uVar8 >> ((byte)iVar12 & 0x1f)) + *(char *)((int)&vc1::g_dbghelp + iVar5 + 3);
//           do {
//             iVar12 = iVar12 + -6;
//             iVar5 = iVar5 + -1;
//             *(byte *)(iVar13 + *(int *)(local_10 + 0x14)) =
//                  ((byte)(uVar8 >> ((byte)iVar12 & 0x1f)) & 0x3f) + 0x80;
//             iVar13 = iVar13 + 1;
//           } while (0 < iVar5);
//           iVar4 = iVar13 + iVar5;
//         }
//         uVar8 = sd->Size;
//       }
//       local_10 = local_10 + 0x20;
//       local_c = local_c + 1;
//     } while (local_c < numFiles);
//   }
//   return 0;
// }
// ABI / bounds, original assembly:
// 00478070 SUB ESP,0x10
// 00478073 CMP dword ptr [ESP + 0x18],0x0
// 00478078 PUSH EBX
// 00478079 PUSH EBP
// 0047807a MOV EBP,dword ptr [ESP + 0x1c]
// 0047807e PUSH ESI
// 0047807f PUSH EDI
// 004782ab POP EDI
// 004782ac POP ESI
// 004782ad POP EBP
// 004782ae MOV EAX,0x8007000e
// 004782b3 POP EBX
// 004782b4 ADD ESP,0x10
// 004782b7 RET
// Incoming reference SzReadHeader2:
// 00479bea MOV EDX,dword ptr [ECX]
// 00479bec MOV EAX,dword ptr [ESP + 0x10]
// 00479bf0 PUSH EDX
// 00479bf1 PUSH EAX
// 00479bf2 MOV EAX,dword ptr [ESP + 0x1c]
// 00479bf6 PUSH EBP
// 00479bf7 CALL 0x00478070
// 00479bfc ADD ESP,0xc
// 00479bff JMP 0x00479d04
// 00479c04 MOV ECX,dword ptr [ESP + 0x54]
// VC9 /GL /LTCG asmcmp 2026-10-22: 98.40%.
// FUNCTION: VC1 0x478070
// SzReadFileNames

// Ghidra VirtualCity.Unwrapped.exe 0x4782c0..004782e2.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadSwitch(_CSzState *param0)
//
// {
//   char cVar1;
//   undefined4 *in_EAX;
//
//   if (in_EAX[1] != 0) {
//     in_EAX[1] = in_EAX[1] + -1;
//     cVar1 = *(char *)*in_EAX;
//     *in_EAX = (char *)*in_EAX + 1;
//     return -(uint)(cVar1 != '\0') & 6;
//   }
//   return 6;
// }
// ABI / bounds, original assembly:
// 004782c0 MOV ECX,dword ptr [EAX + 0x4]
// 004782c3 TEST ECX,ECX
// 004782c5 JZ 0x004782dd
// 004782c7 DEC ECX
// 004782c8 MOV dword ptr [EAX + 0x4],ECX
// 004782cb MOV ECX,dword ptr [EAX]
// 004782cd MOV DL,byte ptr [ECX]
// 004782d2 MOVZX EAX,DL
// 004782d5 NEG EAX
// 004782d7 SBB EAX,EAX
// 004782d9 AND EAX,0x6
// 004782dc RET
// 004782dd MOV EAX,0x6
// 004782e2 RET
// Incoming reference SzReadHeader2:
// 00479bbf JMP 0x00479cfa
// 00479bc4 LEA EAX,[EDI + -0xe]
// 00479bc7 CMP EAX,0x6
// 00479bca JA 0x00479cf0
// 00479bd0 JMP dword ptr [EAX*0x4 + 0x479de0]
// 00479bd7 MOV EAX,EBP
// 00479bd9 CALL 0x004782c0
// 00479bde TEST EAX,EAX
// 00479be0 JNZ 0x00479dd6
// 00479be6 MOV ECX,dword ptr [ESP + 0x50]
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x4782c0
// SzReadSwitch

// Ghidra VirtualCity.Unwrapped.exe 0x4782f0..00478368.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadBoolVector(_CSzState *param0,uint param1,uchar **param2,void *param3)
//
// {
//   byte bVar1;
//   uchar *puVar2;
//   uint uVar3;
//   byte bVar4;
//   undefined4 *unaff_ESI;
//   uint unaff_EDI;
//
//   bVar4 = 0;
//   bVar1 = 0;
//   if (unaff_EDI == 0) {
//     param0->Data = (uchar *)0x0;
//   }
//   else {
//     puVar2 = (uchar *)(*(code *)param1)();
//     param0->Data = puVar2;
//     if (puVar2 == (uchar *)0x0) {
//       return -0x7ff8fff2;
//     }
//   }
//   uVar3 = 0;
//   if (unaff_EDI != 0) {
//     do {
//       if (bVar4 == 0) {
//         if (unaff_ESI[1] == 0) {
//           return 6;
//         }
//         unaff_ESI[1] = unaff_ESI[1] + -1;
//         bVar1 = *(byte *)*unaff_ESI;
//         *unaff_ESI = (byte *)*unaff_ESI + 1;
//         bVar4 = 0x80;
//       }
//       param0->Data[uVar3] = (bVar1 & bVar4) != 0;
//       uVar3 = uVar3 + 1;
//       bVar4 = bVar4 >> 1;
//     } while (uVar3 < unaff_EDI);
//   }
//   return 0;
// }
// ABI / bounds, original assembly:
// 004782f0 PUSH ECX
// 004782f1 PUSH EBX
// 004782f2 XOR BL,BL
// 004782f4 PUSH EBP
// 004782f5 MOV EBP,dword ptr [ESP + 0x10]
// 004782f9 MOV byte ptr [ESP + 0xb],0x0
// 004782fe TEST EDI,EDI
// 0047835e POP ECX
// 0047835f RET
// 00478360 POP EBP
// 00478361 MOV EAX,0x6
// 00478366 POP EBX
// 00478367 POP ECX
// 00478368 RET
// Incoming reference SzReadBoolVector2:
// 004784c7 INC EAX
// 004784c8 MOV dword ptr [ESI],EAX
// 004784ca TEST CL,CL
// 004784cc JNZ 0x004784e3
// 004784ce PUSH EDX
// 004784cf PUSH EBX
// 004784d0 CALL 0x004782f0
// 004784d5 ADD ESP,0x8
// 004784d8 POP EDI
// 004784d9 POP ESI
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x4782f0
// SzReadBoolVector

// Ghidra VirtualCity.Unwrapped.exe 0x478370..00478413.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadNumber(_CSzState *param0,ulonglong *param1)
//
// {
//   uint uVar1;
//   byte bVar2;
//   int iVar3;
//   uint extraout_EAX;
//   uint extraout_EAX_00;
//   uint extraout_EDX;
//   int extraout_EDX_00;
//   undefined4 *unaff_EBX;
//   int iVar4;
//   uint *unaff_ESI;
//   byte *pbVar5;
//   byte local_a;
//
//   iVar4 = 0;
//   local_a = 0x80;
//   if (unaff_EBX[1] == 0) {
//     return 6;
//   }
//   bVar2 = *(byte *)*unaff_EBX;
//   iVar3 = unaff_EBX[1] + -1;
//   pbVar5 = (byte *)*unaff_EBX + 1;
//   unaff_EBX[1] = iVar3;
//   *unaff_EBX = pbVar5;
//   *unaff_ESI = 0;
//   unaff_ESI[1] = 0;
//   while( true ) {
//     if ((local_a & bVar2) == 0) {
//       _allshl();
//       uVar1 = *unaff_ESI;
//       *unaff_ESI = *unaff_ESI + extraout_EAX_00;
//       unaff_ESI[1] = unaff_ESI[1] + extraout_EDX_00 + (uint)CARRY4(uVar1,extraout_EAX_00);
//       return 0;
//     }
//     if (iVar3 == 0) break;
//     iVar3 = iVar3 + -1;
//     unaff_EBX[1] = iVar3;
//     pbVar5 = pbVar5 + 1;
//     *unaff_EBX = pbVar5;
//     _allshl();
//     *unaff_ESI = *unaff_ESI | extraout_EAX;
//     unaff_ESI[1] = unaff_ESI[1] | extraout_EDX;
//     local_a = local_a >> 1;
//     iVar4 = iVar4 + 8;
//     if (0x3f < iVar4) {
//       return 0;
//     }
//   }
//   return 6;
// }
// ABI / bounds, original assembly:
// 00478370 SUB ESP,0xc
// 00478373 MOV EAX,dword ptr [EBX + 0x4]
// 00478376 PUSH EBP
// 00478377 XOR EBP,EBP
// 00478379 MOV DL,0x80
// 0047837b PUSH EDI
// 0047837c MOV byte ptr [ESP + 0xa],DL
// 00478407 ADD dword ptr [ESI],EAX
// 00478409 ADC dword ptr [ESI + 0x4],EDX
// 0047840c POP EDI
// 0047840d XOR EAX,EAX
// 0047840f POP EBP
// 00478410 ADD ESP,0xc
// 00478413 RET
// Incoming reference SzSkeepData:
// 00478520 SUB ESP,0xc
// 00478523 PUSH EBX
// 00478524 PUSH ESI
// 00478525 MOV EBX,EAX
// 00478527 LEA ESI,[ESP + 0xc]
// 0047852b CALL 0x00478370
// 00478530 TEST EAX,EAX
// 00478532 JNZ 0x0047855f
// 00478534 MOV EDX,dword ptr [ESP + 0x10]
// VC9 /GL /LTCG asmcmp 2026-10-22: 86.96%.
// FUNCTION: VC1 0x478370
// SzReadNumber

// Ghidra VirtualCity.Unwrapped.exe 0x478420..0047845d.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadUInt32(_CSzState *param0,uint *param1)
//
// {
//   byte bVar1;
//   undefined4 *in_EAX;
//   byte bVar2;
//   int iVar3;
//   uint *unaff_EDI;
//
//   *unaff_EDI = 0;
//   iVar3 = 0;
//   do {
//     if (in_EAX[1] == 0) {
//       return 6;
//     }
//     in_EAX[1] = in_EAX[1] + -1;
//     bVar1 = *(byte *)*in_EAX;
//     *in_EAX = (byte *)*in_EAX + 1;
//     bVar2 = (byte)iVar3;
//     iVar3 = iVar3 + 8;
//     *unaff_EDI = *unaff_EDI | (uint)bVar1 << (bVar2 & 0x1f);
//   } while (iVar3 < 0x20);
//   return 0;
// }
// ABI / bounds, original assembly:
// 00478420 PUSH ESI
// 00478421 MOV dword ptr [EDI],0x0
// 00478427 XOR ESI,ESI
// 00478429 LEA ESP,[ESP]
// 00478430 MOV ECX,dword ptr [EAX + 0x4]
// 00478433 TEST ECX,ECX
// 00478435 JZ 0x00478457
// 00478451 JL 0x00478430
// 00478453 XOR EAX,EAX
// 00478455 POP ESI
// 00478456 RET
// 00478457 MOV EAX,0x6
// 0047845c POP ESI
// 0047845d RET
// Incoming reference SzReadHashDigests:
// 00478ce4 MOV ECX,dword ptr [EAX]
// 00478ce6 CMP byte ptr [ESI + ECX*0x1],0x0
// 00478cea JZ 0x00478cfe
// 00478cec MOV EDX,dword ptr [EBX]
// 00478cee MOV EAX,dword ptr [ESP + 0x14]
// 00478cf2 LEA EDI,[EDX + ESI*0x4]
// 00478cf5 CALL 0x00478420
// 00478cfa TEST EAX,EAX
// 00478cfc JNZ 0x00478d05
// 00478cfe INC ESI
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x478420
// SzReadUInt32

// Ghidra VirtualCity.Unwrapped.exe 0x478460..00478496.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadBytes(_CSzState *param0,uchar *param1,uint param2)
//
// {
//   undefined1 *puVar1;
//   undefined4 *in_EAX;
//   uint uVar2;
//   uint unaff_EDI;
//
//   uVar2 = 0;
//   if (unaff_EDI != 0) {
//     do {
//       if (in_EAX[1] == 0) {
//         return 6;
//       }
//       puVar1 = (undefined1 *)*in_EAX;
//       in_EAX[1] = in_EAX[1] + -1;
//       *(undefined1 *)((int)&param0->Data + uVar2) = *puVar1;
//       uVar2 = uVar2 + 1;
//       *in_EAX = puVar1 + 1;
//     } while (uVar2 < unaff_EDI);
//   }
//   return 0;
// }
// ABI / bounds, original assembly:
// 00478460 PUSH EBP
// 00478461 MOV EBP,dword ptr [ESP + 0x8]
// 00478465 XOR ECX,ECX
// 00478467 PUSH ESI
// 00478468 TEST EDI,EDI
// 0047846a JBE 0x0047848a
// 0047846c LEA ESP,[ESP]
// 0047848b XOR EAX,EAX
// 0047848d POP EBP
// 0047848e RET
// 0047848f POP ESI
// 00478490 MOV EAX,0x6
// 00478495 POP EBP
// 00478496 RET
// Incoming reference SzGetNextFolderItem:
// 0047891b MOV EAX,ECX
// 0047891d CMP EAX,EBP
// 0047891f JZ 0x004788b4
// 00478921 MOV EDX,dword ptr [ESI + 0x14]
// 00478924 PUSH EDX
// 00478925 MOV EAX,EBX
// 00478927 CALL 0x00478460
// 0047892c ADD ESP,0x4
// 0047892f CMP EAX,EBP
// 00478931 JNZ 0x00478c09
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x478460
// SzReadBytes

// Ghidra VirtualCity.Unwrapped.exe 0x4784a0..0x4784ae.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SafeReadDirectByte(_ISzInStream *param0,uchar *param1)
//
// {
//   _ISzInStream *in_EAX;
//   int iVar1;
//   uchar *unaff_retaddr;
//
//   iVar1 = SafeReadDirect(in_EAX,unaff_retaddr,(uint)param0);
//   return iVar1;
// }
// ABI / bounds, original assembly:
// 0x4784a0 push eax
// 0x4784a1 mov eax, 1
// 0x4784a6 call 0x477e00
// 0x4784ab add esp, 4
// 0x4784ae ret
// 0x4784a0 push eax
// 0x4784a1 mov eax, 1
// 0x4784a6 call 0x477e00
// 0x4784ab add esp, 4
// 0x4784ae ret
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x4784a0
// SafeReadDirectByte

// Ghidra VirtualCity.Unwrapped.exe 0x4784b0..00478513.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadBoolVector2(_CSzState *sd,uint param1,uchar **param2,void *param3)
//
// {
//   uchar uVar1;
//   uint in_EAX;
//   int iVar2;
//   uint uVar3;
//   uchar *puVar4;
//   code *in_EDX;
//   _CSzState *unaff_EBX;
//   void *unaff_ESI;
//   uchar **unaff_EDI;
//
//   if (sd->Size == 0) {
//     return 6;
//   }
//   sd->Size = sd->Size - 1;
//   uVar1 = *sd->Data;
//   sd->Data = sd->Data + 1;
//   if (uVar1 == '\0') {
//     iVar2 = SzReadBoolVector(unaff_EBX,(uint)in_EDX,unaff_EDI,unaff_ESI);
//     return iVar2;
//   }
//   if (in_EAX == 0) {
//     unaff_EBX->Data = (uchar *)0x0;
//   }
//   else {
//     puVar4 = (uchar *)(*in_EDX)();
//     unaff_EBX->Data = puVar4;
//     if (puVar4 == (uchar *)0x0) {
//       return -0x7ff8fff2;
//     }
//   }
//   uVar3 = 0;
//   if (in_EAX != 0) {
//     do {
//       unaff_EBX->Data[uVar3] = '\x01';
//       uVar3 = uVar3 + 1;
//     } while (uVar3 < in_EAX);
//   }
//   return 0;
// }
// ABI / bounds, original assembly:
// 004784b0 PUSH ESI
// 004784b1 MOV ESI,dword ptr [ESP + 0x8]
// 004784b5 PUSH EDI
// 004784b6 MOV EDI,EAX
// 004784b8 MOV EAX,dword ptr [ESI + 0x4]
// 004784bb TEST EAX,EAX
// 004784bd JZ 0x004784db
// 00478506 MOV dword ptr [EBX],EAX
// 00478508 TEST EAX,EAX
// 0047850a JNZ 0x004784e9
// 0047850c POP EDI
// 0047850d MOV EAX,0x8007000e
// 00478512 POP ESI
// 00478513 RET
// Incoming reference SzReadHashDigests:
// 00478c8a PUSH ESI
// 00478c8b MOV ESI,EAX
// 00478c8d MOV EAX,dword ptr [ESP + 0x10]
// 00478c91 PUSH EAX
// 00478c92 MOV EDX,ESI
// 00478c94 MOV EAX,EBP
// 00478c96 CALL 0x004784b0
// 00478c9b ADD ESP,0x4
// 00478c9e TEST EAX,EAX
// 00478ca0 JNZ 0x00478d06
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x4784b0
// SzReadBoolVector2

// Ghidra VirtualCity.Unwrapped.exe 0x478520..00478564.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzSkeepData(_CSzState *param0)
//
// {
//   int *in_EAX;
//   int iVar1;
//   ulonglong *unaff_EBX;
//   _CSzState *unaff_ESI;
//   uint local_8;
//   int local_4;
//
//   iVar1 = SzReadNumber(unaff_ESI,unaff_EBX);
//   if (iVar1 == 0) {
//     if ((local_4 != 0) || ((uint)in_EAX[1] < local_8)) {
//       return 6;
//     }
//     *in_EAX = *in_EAX + local_8;
//     in_EAX[1] = in_EAX[1] - local_8;
//     iVar1 = 0;
//   }
//   return iVar1;
// }
// ABI / bounds, original assembly:
// 00478520 SUB ESP,0xc
// 00478523 PUSH EBX
// 00478524 PUSH ESI
// 00478525 MOV EBX,EAX
// 00478527 LEA ESI,[ESP + 0xc]
// 0047852b CALL 0x00478370
// 00478530 TEST EAX,EAX
// 00478558 SUB EAX,ECX
// 0047855a MOV dword ptr [EBX + 0x4],EAX
// 0047855d XOR EAX,EAX
// 0047855f POP ESI
// 00478560 POP EBX
// 00478561 ADD ESP,0xc
// 00478564 RET
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 76.67%.
// FUNCTION: VC1 0x478520
// SzSkeepData

// Ghidra VirtualCity.Unwrapped.exe 0x478570..0x478577.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadID(void)
//
// {
//   int iVar1;
//   _CSzState *in_ECX;
//   ulonglong *unaff_retaddr;
//
//   iVar1 = SzReadNumber(in_ECX,unaff_retaddr);
//   return iVar1;
// }
// ABI / bounds, original assembly:
// 0x478570 push ecx
// 0x478571 call 0x478370
// 0x478576 pop ecx
// 0x478577 ret
// 0x478570 push ecx
// 0x478571 call 0x478370
// 0x478576 pop ecx
// 0x478577 ret
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 0.00%.
// FUNCTION: VC1 0x478570
// SzReadID

// Ghidra VirtualCity.Unwrapped.exe 0x478580..004785c7.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadNumber32(_CSzState *param0,uint *param1)
//
// {
//   int iVar1;
//   _CSzState *unaff_ESI;
//   ulonglong *in_stack_fffffff4;
//   uchar *local_8;
//   int local_4;
//
//   iVar1 = SzReadNumber(unaff_ESI,in_stack_fffffff4);
//   if (iVar1 == 0) {
//     if (((local_4 != 0) || ((uchar *)0x7fffffff < local_8)) || ((uchar *)0x3ffffff < local_8)) {
//       return -0x7fffbfff;
//     }
//     param0->Data = local_8;
//     iVar1 = 0;
//   }
//   return iVar1;
// }
// ABI / bounds, original assembly:
// 00478580 SUB ESP,0xc
// 00478583 PUSH ESI
// 00478584 LEA ESI,[ESP + 0x8]
// 00478588 CALL 0x00478370
// 0047858d TEST EAX,EAX
// 0047858f JNZ 0x004785c3
// 00478591 MOV EAX,dword ptr [ESP + 0xc]
// 004785b9 JNC 0x004785a5
// 004785bb MOV EAX,dword ptr [ESP + 0x14]
// 004785bf MOV dword ptr [EAX],ECX
// 004785c1 XOR EAX,EAX
// 004785c3 POP ESI
// 004785c4 ADD ESP,0xc
// 004785c7 RET
// Incoming reference SzReadUnPackInfo:
// 00479312 CALL 0x00478d10
// 00479317 ADD ESP,0x8
// 0047931a CMP EAX,EDI
// 0047931c JNZ 0x004795f5
// 00479322 MOV EBP,dword ptr [ESP + 0x38]
// 00479326 PUSH EBP
// 00479327 CALL 0x00478580
// 0047932c ADD ESP,0x4
// 0047932f CMP EAX,EDI
// 00479331 JNZ 0x004795f5
// VC9 /GL /LTCG asmcmp 2026-10-22: 44.90%.
// FUNCTION: VC1 0x478580
// SzReadNumber32

// Ghidra VirtualCity.Unwrapped.exe 0x4785d0..004785f4.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadSize(_CSzState *param0,ulonglong *param1)
//
// {
//   int iVar1;
//   _CSzState *unaff_ESI;
//   undefined4 *unaff_EDI;
//   ulonglong *in_stack_fffffff8;
//   undefined4 local_4;
//
//   iVar1 = SzReadNumber(unaff_ESI,in_stack_fffffff8);
//   if (iVar1 == 0) {
//     *unaff_EDI = in_stack_fffffff8;
//     unaff_EDI[1] = local_4;
//     iVar1 = 0;
//   }
//   return iVar1;
// }
// ABI / bounds, original assembly:
// 004785d0 SUB ESP,0x8
// 004785d3 PUSH ESI
// 004785d4 LEA ESI,[ESP + 0x4]
// 004785d8 CALL 0x00478370
// 004785dd TEST EAX,EAX
// 004785df JNZ 0x004785f0
// 004785e1 MOV EAX,dword ptr [ESP + 0x4]
// 004785e5 MOV ECX,dword ptr [ESP + 0x8]
// 004785e9 MOV dword ptr [EDI],EAX
// 004785eb MOV dword ptr [EDI + 0x4],ECX
// 004785ee XOR EAX,EAX
// 004785f0 POP ESI
// 004785f1 ADD ESP,0x8
// 004785f4 RET
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 66.67%.
// FUNCTION: VC1 0x4785d0
// SzReadSize

// Ghidra VirtualCity.Unwrapped.exe 0x478600..00478666.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SafeReadDirectUInt64(_ISzInStream *inStream,ulonglong *param1,uint *param2)
//
// {
//   ulonglong uVar1;
//   int iVar2;
//   uint extraout_EAX;
//   uint in_ECX;
//   uint extraout_EDX;
//   int iVar3;
//   uint unaff_EBP;
//   uint *unaff_ESI;
//   uchar *unaff_EDI;
//
//   iVar3 = 0;
//   *unaff_ESI = 0;
//   unaff_ESI[1] = 0;
//   do {
//     iVar2 = SafeReadDirect(inStream,unaff_EDI,unaff_EBP);
//     if (iVar2 != 0) {
//       return iVar2;
//     }
//     _allshl();
//     *unaff_ESI = *unaff_ESI | extraout_EAX;
//     uVar1 = *param1;
//     unaff_ESI[1] = unaff_ESI[1] | extraout_EDX;
//     iVar3 = iVar3 + 8;
//     *(uint *)param1 =
//          (uint)uVar1 >> 8 ^ *(uint *)(&DAT_004d06a8 + (((uint)uVar1 ^ in_ECX >> 0x18) & 0xff) * 4);
//   } while (iVar3 < 0x40);
//   return 0;
// }
// ABI / bounds, original assembly:
// 00478600 PUSH ECX
// 00478601 PUSH EBX
// 00478602 PUSH EBP
// 00478603 MOV EBP,dword ptr [ESP + 0x14]
// 00478607 XOR EBX,EBX
// 00478609 PUSH EDI
// 0047860a MOV dword ptr [ESI],EBX
// 0047865e JL 0x00478610
// 00478660 XOR EAX,EAX
// 00478662 POP EDI
// 00478663 POP EBP
// 00478664 POP EBX
// 00478665 POP ECX
// 00478666 RET
// Incoming reference SzArchiveOpen2:
// 00479ff7 JNZ 0x0047a1f1
// 00479ffd MOV EAX,EBX
// 00479fff PUSH EAX
// 0047a000 PUSH ESI
// 0047a001 LEA ESI,[ESP + 0x38]
// 0047a005 MOV dword ptr [ESP + 0x18],0xffffffff
// 0047a00d CALL 0x00478600
// 0047a012 ADD ESP,0x8
// 0047a015 TEST EAX,EAX
// 0047a017 JNZ 0x0047a1f1
// VC9 /GL /LTCG asmcmp 2026-10-22: 97.50%.
// FUNCTION: VC1 0x478600
// SafeReadDirectUInt64

// Ghidra VirtualCity.Unwrapped.exe 0x478670..004786c8.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SafeReadDirectUInt32(_ISzInStream *inStream,uint *param1,uint *param2)
//
// {
//   int iVar1;
//   byte bVar2;
//   uint in_ECX;
//   uint *unaff_EBX;
//   uint unaff_EBP;
//   uchar *unaff_ESI;
//   int iVar3;
//   uint *unaff_EDI;
//
//   *unaff_EDI = 0;
//   iVar3 = 0;
//   do {
//     iVar1 = SafeReadDirect(inStream,unaff_ESI,unaff_EBP);
//     if (iVar1 != 0) {
//       return iVar1;
//     }
//     bVar2 = (byte)iVar3;
//     iVar3 = iVar3 + 8;
//     *unaff_EDI = *unaff_EDI | (in_ECX >> 0x18) << (bVar2 & 0x1f);
//     *unaff_EBX = *unaff_EBX >> 8 ^
//                  *(uint *)(&DAT_004d06a8 + ((*unaff_EBX ^ in_ECX >> 0x18) & 0xff) * 4);
//   } while (iVar3 < 0x20);
//   return 0;
// }
// ABI / bounds, original assembly:
// 00478670 PUSH ECX
// 00478671 PUSH EBP
// 00478672 MOV EBP,dword ptr [ESP + 0xc]
// 00478676 PUSH ESI
// 00478677 MOV dword ptr [EDI],0x0
// 0047867d XOR ESI,ESI
// 0047867f NOP
// 004786bf MOV dword ptr [EBX],EAX
// 004786c1 JL 0x00478680
// 004786c3 XOR EAX,EAX
// 004786c5 POP ESI
// 004786c6 POP EBP
// 004786c7 POP ECX
// 004786c8 RET
// Incoming reference SzArchiveOpen2:
// 00479fd9 ADD ESP,0x4
// 00479fdc TEST EAX,EAX
// 00479fde JNZ 0x0047a1f1
// 00479fe4 PUSH ESI
// 00479fe5 LEA EBX,[ESP + 0x14]
// 00479fe9 LEA EDI,[ESP + 0x1c]
// 00479fed CALL 0x00478670
// 00479ff2 ADD ESP,0x4
// 00479ff5 TEST EAX,EAX
// 00479ff7 JNZ 0x0047a1f1
// VC9 /GL /LTCG asmcmp 2026-10-22: 97.06%.
// FUNCTION: VC1 0x478670
// SafeReadDirectUInt32

// Ghidra VirtualCity.Unwrapped.exe 0x4786d0..00478c7a.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// /* WARNING: Removing unreachable block (ram,0x00478906) */
//
// int SzGetNextFolderItem(_CSzState *param0,_CFolder *param1,void *param2)
//
// {
//   undefined4 *puVar1;
//   byte bVar2;
//   int *in_EAX;
//   int iVar3;
//   uint *puVar4;
//   uint extraout_EAX;
//   uchar *puVar5;
//   uint extraout_EDX;
//   uint *puVar6;
//   uint uVar7;
//   ulonglong *unaff_ESI;
//   byte *pbVar8;
//   uint uVar9;
//   uchar *puVar10;
//   _CSzState *unaff_EDI;
//   int local_64;
//   int local_60;
//   uint local_5c;
//   uint local_58;
//   uchar *local_54;
//   uchar *local_40;
//   int local_3c;
//   uint local_38;
//   int local_34;
//   uint local_30;
//   int local_2c;
//   uint local_28;
//   int local_24;
//   uint local_20;
//   int local_1c;
//   byte abStack_10 [16];
//
//   local_5c = 0;
//   local_60 = 0;
//   iVar3 = SzReadNumber(unaff_EDI,unaff_ESI);
//   if (iVar3 != 0) {
//     return iVar3;
//   }
//   if (((local_3c != 0) || ((uchar *)0x7fffffff < local_40)) || ((uchar *)0x3ffffff < local_40)) {
//     return -0x7fffbfff;
//   }
//   param0->Data = local_40;
//   if (local_40 == (uchar *)0x0) {
//     param0->Size = 0;
//   }
//   else {
//     uVar7 = (*(code *)param1)((int)local_40 * 0x18);
//     param0->Size = uVar7;
//     if (uVar7 == 0) {
//       return -0x7ff8fff2;
//     }
//   }
//   if (local_40 != (uchar *)0x0) {
//     iVar3 = 0;
//     puVar5 = local_40;
//     do {
//       puVar1 = (undefined4 *)(iVar3 + 0x10 + param0->Size);
//       iVar3 = iVar3 + 0x18;
//       puVar5 = puVar5 + -1;
//       *puVar1 = 0;
//       puVar1[1] = 0;
//     } while (puVar5 != (uchar *)0x0);
//   }
//   local_54 = (uchar *)0x0;
//   if (local_40 != (uchar *)0x0) {
//     local_64 = 0;
//     do {
//       puVar4 = (uint *)(param0->Size + local_64);
//       if (in_EAX[1] == 0) {
//         return 6;
//       }
//       bVar2 = *(byte *)*in_EAX;
//       iVar3 = in_EAX[1] + -1;
//       pbVar8 = (byte *)*in_EAX + 1;
//       local_58 = bVar2 & 0xf;
//       in_EAX[1] = iVar3;
//       *in_EAX = (int)pbVar8;
//       uVar7 = 0;
//       if ((bVar2 & 0xf) != 0) {
//         do {
//           if (iVar3 == 0) {
//             return 6;
//           }
//           abStack_10[uVar7] = *pbVar8;
//           iVar3 = iVar3 + -1;
//           pbVar8 = pbVar8 + 1;
//           uVar7 = uVar7 + 1;
//           in_EAX[1] = iVar3;
//           *in_EAX = (int)pbVar8;
//         } while (uVar7 < local_58);
//       }
//       if (8 < local_58) {
//         return -0x7fffbfff;
//       }
//       puVar4[2] = 0;
//       puVar4[3] = 0;
//       if ((bVar2 & 0xf) != 0) {
//         do {
//           _allshl();
//           puVar4[2] = puVar4[2] | extraout_EAX;
//           puVar4[3] = puVar4[3] | extraout_EDX;
//           local_58 = local_58 - 1;
//         } while (local_58 != 0);
//       }
//       if ((bVar2 & 0x10) == 0) {
//         *puVar4 = 1;
//         puVar4[1] = 1;
//       }
//       else {
//         iVar3 = SzReadNumber(unaff_EDI,unaff_ESI);
//         if (iVar3 != 0) {
//           return iVar3;
//         }
//         if (local_34 != 0) {
//           return -0x7fffbfff;
//         }
//         if (0x7fffffff < local_38) {
//           return -0x7fffbfff;
//         }
//         if (0x3ffffff < local_38) {
//           return -0x7fffbfff;
//         }
//         *puVar4 = local_38;
//         iVar3 = SzReadNumber(unaff_EDI,unaff_ESI);
//         if (iVar3 != 0) {
//           return iVar3;
//         }
//         if (local_2c != 0) {
//           return -0x7fffbfff;
//         }
//         if (0x7fffffff < local_30) {
//           return -0x7fffbfff;
//         }
//         if (0x3ffffff < local_30) {
//           return -0x7fffbfff;
//         }
//         puVar4[1] = local_30;
//       }
//       if ((bVar2 & 0x20) != 0) {
//         iVar3 = SzReadNumber(unaff_EDI,unaff_ESI);
//         if (iVar3 != 0) {
//           return iVar3;
//         }
//         puVar4[4] = 0;
//         puVar4[5] = 0;
//         iVar3 = SzReadBytes((_CSzState *)puVar4[5],(uchar *)unaff_EDI,(uint)unaff_ESI);
//         if (iVar3 != 0) {
//           return iVar3;
//         }
//       }
//       while ((char)bVar2 < '\0') {
//         if (in_EAX[1] == 0) {
//           return 6;
//         }
//         bVar2 = *(byte *)*in_EAX;
//         pbVar8 = (byte *)*in_EAX + 1;
//         uVar7 = (uint)(bVar2 & 0xf);
//         uVar9 = in_EAX[1] - 1;
//         in_EAX[1] = uVar9;
//         *in_EAX = (int)pbVar8;
//         if (uVar9 < uVar7) {
//           return 6;
//         }
//         in_EAX[1] = uVar9 - uVar7;
//         *in_EAX = (int)(pbVar8 + uVar7);
//         if ((bVar2 & 0x10) != 0) {
//           iVar3 = SzReadNumber(unaff_EDI,unaff_ESI);
//           if (iVar3 != 0) {
//             return iVar3;
//           }
//           if (local_24 != 0) {
//             return -0x7fffbfff;
//           }
//           if (0x7fffffff < local_28) {
//             return -0x7fffbfff;
//           }
//           if (0x3ffffff < local_28) {
//             return -0x7fffbfff;
//           }
//           iVar3 = SzReadNumber(unaff_EDI,unaff_ESI);
//           if (iVar3 != 0) {
//             return iVar3;
//           }
//           if (local_1c != 0) {
//             return -0x7fffbfff;
//           }
//           if (0x7fffffff < local_20) {
//             return -0x7fffbfff;
//           }
//           if (0x3ffffff < local_20) {
//             return -0x7fffbfff;
//           }
//         }
//         if ((bVar2 & 0x20) != 0) {
//           iVar3 = SzReadNumber(unaff_EDI,unaff_ESI);
//           if (iVar3 != 0) {
//             return iVar3;
//           }
//           *in_EAX = *in_EAX;
//           in_EAX[1] = in_EAX[1];
//         }
//       }
//       local_5c = local_5c + *puVar4;
//       local_60 = local_60 + puVar4[1];
//       local_64 = local_64 + 0x18;
//       local_54 = local_54 + 1;
//     } while (local_54 < local_40);
//   }
//   puVar10 = (uchar *)(local_60 - 1);
//   puVar5 = (uchar *)0x0;
//   param0[1].Data = puVar10;
//   if (puVar10 == (uchar *)0x0) {
//     param0[1].Size = 0;
//   }
//   else {
//     uVar7 = (*(code *)param1)((int)puVar10 * 8);
//     param0[1].Size = uVar7;
//     if (uVar7 == 0) {
//       return -0x7ff8fff2;
//     }
//   }
//   if (puVar10 != (uchar *)0x0) {
//     do {
//       puVar4 = (uint *)(param0[1].Size + (int)puVar5 * 8);
//       iVar3 = SzReadNumber(unaff_EDI,unaff_ESI);
//       if (iVar3 != 0) {
//         return iVar3;
//       }
//       if (local_1c != 0) {
//         return -0x7fffbfff;
//       }
//       if (0x7fffffff < local_20) {
//         return -0x7fffbfff;
//       }
//       if (0x3ffffff < local_20) {
//         return -0x7fffbfff;
//       }
//       *puVar4 = local_20;
//       iVar3 = SzReadNumber(unaff_EDI,unaff_ESI);
//       if (iVar3 != 0) {
//         return iVar3;
//       }
//       if (local_24 != 0) {
//         return -0x7fffbfff;
//       }
//       if (0x7fffffff < local_28) {
//         return -0x7fffbfff;
//       }
//       if (0x3ffffff < local_28) {
//         return -0x7fffbfff;
//       }
//       puVar5 = puVar5 + 1;
//       puVar4[1] = local_28;
//     } while (puVar5 < puVar10);
//   }
//   puVar10 = (uchar *)(local_5c - (int)puVar10);
//   param0[2].Data = puVar10;
//   if (puVar10 == (uchar *)0x0) {
//     param0[2].Size = 0;
//   }
//   else {
//     puVar4 = (uint *)(*(code *)param1)((int)puVar10 * 4);
//     param0[2].Size = (uint)puVar4;
//     if (puVar4 == (uint *)0x0) {
//       return -0x7ff8fff2;
//     }
//     if (puVar10 == (uchar *)0x1) {
//       uVar7 = 0;
//       if (local_5c == 0) {
//         return 0;
//       }
//       while (puVar5 = (uchar *)0x0, param0[1].Data != (uchar *)0x0) {
//         puVar6 = (uint *)param0[1].Size;
//         while (*puVar6 != uVar7) {
//           puVar5 = puVar5 + 1;
//           puVar6 = puVar6 + 2;
//           if (param0[1].Data <= puVar5) goto LAB_00478c5c;
//         }
//         if ((int)puVar5 < 0) break;
//         uVar7 = uVar7 + 1;
//         if (local_5c <= uVar7) {
//           return 0;
//         }
//       }
// LAB_00478c5c:
//       *puVar4 = uVar7;
//       return 0;
//     }
//   }
//   puVar5 = (uchar *)0x0;
//   if (puVar10 != (uchar *)0x0) {
//     do {
//       iVar3 = SzReadNumber(unaff_EDI,unaff_ESI);
//       if (iVar3 != 0) {
//         return iVar3;
//       }
//       if (local_1c != 0) {
//         return -0x7fffbfff;
//       }
//       if (0x7fffffff < local_20) {
//         return -0x7fffbfff;
//       }
//       if (0x3ffffff < local_20) {
//         return -0x7fffbfff;
//       }
//       *(uint *)(param0[2].Size + (int)puVar5 * 4) = local_20;
//       puVar5 = puVar5 + 1;
//     } while (puVar5 < puVar10);
//   }
//   return 0;
// }
// ABI / bounds, original assembly:
// 004786d0 SUB ESP,0x6c
// 004786d3 PUSH EBX
// 004786d4 PUSH EBP
// 004786d5 PUSH ESI
// 004786d6 XOR EBP,EBP
// 004786d8 PUSH EDI
// 004786d9 MOV EDI,dword ptr [ESP + 0x80]
// 00478c71 POP EDI
// 00478c72 POP ESI
// 00478c73 POP EBP
// 00478c74 XOR EAX,EAX
// 00478c76 POP EBX
// 00478c77 ADD ESP,0x6c
// 00478c7a RET
// Incoming reference SzReadUnPackInfo:
// 004793e0 MOV EDX,dword ptr [ESP + 0x40]
// 004793e4 MOV EAX,dword ptr [ESI]
// 004793e6 ADD EAX,EBP
// 004793e8 PUSH EDX
// 004793e9 PUSH EAX
// 004793ea MOV EAX,EBX
// 004793ec CALL 0x004786d0
// 004793f1 ADD ESP,0x8
// 004793f4 CMP EAX,EDI
// 004793f6 JNZ 0x004795f5
// VC9 /GL /LTCG asmcmp 2026-10-22: 99.11%.
// FUNCTION: VC1 0x4786d0
// SzGetNextFolderItem

// Ghidra VirtualCity.Unwrapped.exe 0x478c80..00478d09.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadHashDigests(_CSzState *sd,uint numItems,uchar **digestsDefined,uint **digests,void *param4
//                      )
//
// {
//   code *in_EAX;
//   int iVar1;
//   uint *puVar2;
//   void *unaff_EBX;
//   uchar **unaff_EBP;
//   uint *unaff_ESI;
//   uint uVar3;
//   _CSzState *unaff_EDI;
//
//   iVar1 = SzReadBoolVector2(sd,(uint)unaff_ESI,unaff_EBP,unaff_EBX);
//   if (iVar1 == 0) {
//     if (numItems == 0) {
//       *digests = (uint *)0x0;
//     }
//     else {
//       puVar2 = (uint *)(*in_EAX)(numItems * 4);
//       *digests = puVar2;
//       if (puVar2 == (uint *)0x0) {
//         return -0x7ff8fff2;
//       }
//     }
//     uVar3 = 0;
//     if (numItems != 0) {
//       do {
//         if (((*digestsDefined)[uVar3] != '\0') &&
//            (iVar1 = SzReadUInt32(unaff_EDI,unaff_ESI), iVar1 != 0)) {
//           return iVar1;
//         }
//         uVar3 = uVar3 + 1;
//       } while (uVar3 < numItems);
//     }
//     iVar1 = 0;
//   }
//   return iVar1;
// }
// ABI / bounds, original assembly:
// 00478c80 PUSH EBX
// 00478c81 MOV EBX,dword ptr [ESP + 0x10]
// 00478c85 PUSH EBP
// 00478c86 MOV EBP,dword ptr [ESP + 0x10]
// 00478c8a PUSH ESI
// 00478c8b MOV ESI,EAX
// 00478c8d MOV EAX,dword ptr [ESP + 0x10]
// 00478d01 JC 0x00478ce0
// 00478d03 XOR EAX,EAX
// 00478d05 POP EDI
// 00478d06 POP ESI
// 00478d07 POP EBP
// 00478d08 POP EBX
// 00478d09 RET
// Incoming reference SzReadSubStreamsInfo:
// 004791bc MOV EAX,dword ptr [EDX]
// 004791be PUSH ECX
// 004791bf XOR ESI,ESI
// 004791c1 PUSH EBX
// 004791c2 MOV dword ptr [ESP + 0x2c],ESI
// 004791c6 MOV dword ptr [ESP + 0x5c],ESI
// 004791ca CALL 0x00478c80
// 004791cf MOV EDX,dword ptr [ESP + 0x2c]
// 004791d3 MOV EBP,EAX
// 004791d5 ADD ESP,0x10
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x478c80
// SzReadHashDigests

// Ghidra VirtualCity.Unwrapped.exe 0x478d10..00478da7.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzWaitAttribute(_CSzState *param0,ulonglong param1)
//
// {
//   int *in_EAX;
//   int iVar1;
//   ulonglong *unaff_ESI;
//   _CSzState *unaff_EDI;
//   int in_stack_00000008;
//   _CSzState *local_10;
//   int local_c;
//   uint local_8;
//   int local_4;
//
//   iVar1 = SzReadNumber(unaff_EDI,unaff_ESI);
//   if (iVar1 == 0) {
//     while( true ) {
//       if ((local_10 == param0) && (local_c == in_stack_00000008)) {
//         return 0;
//       }
//       if (local_10 == (_CSzState *)0x0 && local_c == 0) break;
//       iVar1 = SzReadNumber(unaff_EDI,unaff_ESI);
//       if (iVar1 != 0) {
//         return iVar1;
//       }
//       if ((local_4 == 0) && (local_8 <= (uint)in_EAX[1])) {
//         *in_EAX = *in_EAX + local_8;
//         in_EAX[1] = in_EAX[1] - local_8;
//         iVar1 = 0;
//       }
//       else {
//         iVar1 = 6;
//       }
//       if (iVar1 != 0) {
//         return iVar1;
//       }
//       iVar1 = SzReadNumber(unaff_EDI,unaff_ESI);
//       if (iVar1 != 0) {
//         return iVar1;
//       }
//     }
//     iVar1 = 6;
//   }
//   return iVar1;
// }
// ABI / bounds, original assembly:
// 00478d10 SUB ESP,0x14
// 00478d13 PUSH EBX
// 00478d14 PUSH EBP
// 00478d15 PUSH ESI
// 00478d16 PUSH EDI
// 00478d17 MOV EBX,EAX
// 00478d19 LEA ESI,[ESP + 0x14]
// 00478d9b MOV EAX,0x6
// 00478da0 POP EDI
// 00478da1 POP ESI
// 00478da2 POP EBP
// 00478da3 POP EBX
// 00478da4 ADD ESP,0x14
// 00478da7 RET
// Incoming reference SzReadUnPackInfo:
// 00479309 PUSH ESI
// 0047930a PUSH EDI
// 0047930b XOR EDI,EDI
// 0047930d PUSH EDI
// 0047930e PUSH 0xb
// 00479310 MOV EAX,EBX
// 00479312 CALL 0x00478d10
// 00479317 ADD ESP,0x8
// 0047931a CMP EAX,EDI
// 0047931c JNZ 0x004795f5
// VC9 /GL /LTCG asmcmp 2026-10-22: 78.79%.
// FUNCTION: VC1 0x478d10
// SzWaitAttribute

// Ghidra VirtualCity.Unwrapped.exe 0x478db0..00478e13.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadArchiveProperties(_CSzState *param0)
//
// {
//   int *in_EAX;
//   int iVar1;
//   ulonglong *unaff_EBX;
//   _CSzState *unaff_ESI;
//   int local_10;
//   int local_c;
//   uint local_8;
//   int local_4;
//
//   iVar1 = SzReadNumber(unaff_ESI,unaff_EBX);
//   if (iVar1 == 0) {
//     while (local_10 != 0 || local_c != 0) {
//       iVar1 = SzReadNumber(unaff_ESI,unaff_EBX);
//       if (((iVar1 == 0) && (local_4 == 0)) && (local_8 <= (uint)in_EAX[1])) {
//         *in_EAX = *in_EAX + local_8;
//         in_EAX[1] = in_EAX[1] - local_8;
//       }
//       iVar1 = SzReadNumber(unaff_ESI,unaff_EBX);
//       if (iVar1 != 0) {
//         return iVar1;
//       }
//     }
//     iVar1 = 0;
//   }
//   return iVar1;
// }
// ABI / bounds, original assembly:
// 00478db0 SUB ESP,0x14
// 00478db3 PUSH EBX
// 00478db4 PUSH ESI
// 00478db5 MOV EBX,EAX
// 00478db7 LEA ESI,[ESP + 0xc]
// 00478dbb CALL 0x00478370
// 00478dc0 TEST EAX,EAX
// 00478e08 ADD ESP,0x14
// 00478e0b RET
// 00478e0c XOR EAX,EAX
// 00478e0e POP ESI
// 00478e0f POP EBX
// 00478e10 ADD ESP,0x14
// 00478e13 RET
// Incoming reference SzReadHeader2:
// 00479a2c MOV EAX,dword ptr [ESP + 0x1c]
// 00479a30 CMP EAX,0x2
// 00479a33 JNZ 0x00479a5c
// 00479a35 CMP dword ptr [ESP + 0x20],0x0
// 00479a3a JNZ 0x00479a5c
// 00479a3c MOV EAX,EBP
// 00479a3e CALL 0x00478db0
// 00479a43 TEST EAX,EAX
// 00479a45 JNZ 0x00479dd6
// 00479a4b CALL 0x00478370
// VC9 /GL /LTCG asmcmp 2026-10-22: 75.00%.
// FUNCTION: VC1 0x478db0
// SzReadArchiveProperties

// Ghidra VirtualCity.Unwrapped.exe 0x478e70..004792fb.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadSubStreamsInfo
//               (_CSzState *sd,uint numFolders,_CFolder *folders,uint *numUnPackStreams,
//               ulonglong **unPackSizes,uchar **digestsDefined,uint **digests,_ISzAlloc *allocTemp)
//
// {
//   longlong lVar1;
//   _ISzAlloc *p_Var2;
//   uint *puVar3;
//   int iVar4;
//   ulonglong *puVar5;
//   uchar *puVar6;
//   uint *puVar7;
//   int *piVar8;
//   int iVar9;
//   int *piVar10;
//   uint uVar11;
//   ulonglong *unaff_ESI;
//   int iVar12;
//   _CFolder *unaff_EDI;
//   uint uVar13;
//   bool bVar14;
//   ulonglong uVar15;
//   int local_2c;
//   _CFolder *local_28;
//   uint local_24;
//   int *local_20;
//   int local_1c;
//   int local_18;
//   int local_14;
//   uint local_10;
//   int local_c;
//   uint local_8;
//   int local_4;
//
//   puVar7 = numUnPackStreams;
//   local_18 = 0;
//   local_14 = 0;
//   local_2c = 0;
//   local_24 = 0;
//   if (numFolders != 0) {
//     puVar3 = &folders->NumUnPackStreams;
//     uVar11 = numFolders;
//     do {
//       *puVar3 = 1;
//       puVar3 = puVar3 + 10;
//       uVar11 = uVar11 - 1;
//     } while (uVar11 != 0);
//   }
//   *numUnPackStreams = numFolders;
//   iVar4 = SzReadNumber((_CSzState *)unaff_EDI,unaff_ESI);
//   iVar9 = local_18;
//   iVar12 = local_14;
//   do {
//     if (iVar4 != 0) {
//       return iVar4;
//     }
//     local_18 = iVar9;
//     local_14 = iVar12;
//     if ((iVar9 == 0xd) && (iVar12 == 0)) {
//       uVar11 = 0;
//       *numUnPackStreams = 0;
//       if (numFolders != 0) {
//         puVar7 = &folders->NumUnPackStreams;
//         do {
//           iVar4 = SzReadNumber((_CSzState *)unaff_EDI,unaff_ESI);
//           if (iVar4 != 0) {
//             return iVar4;
//           }
//           if (((local_c != 0) || (0x7fffffff < local_10)) || (0x3ffffff < local_10)) {
//             return -0x7fffbfff;
//           }
//           *numUnPackStreams = *numUnPackStreams + local_10;
//           *puVar7 = local_10;
//           uVar11 = uVar11 + 1;
//           puVar7 = puVar7 + 10;
//         } while (uVar11 < numFolders);
//       }
//     }
//     else {
//       if (((iVar9 == 10) && (iVar12 == 0)) ||
//          (((iVar9 == 9 && (iVar12 == 0)) || (iVar9 == 0 && iVar12 == 0)))) {
//         if (*puVar7 == 0) {
//           *unPackSizes = (ulonglong *)0x0;
//           *digestsDefined = (uchar *)0x0;
//           *digests = (uint *)0x0;
//         }
//         else {
//           puVar5 = (ulonglong *)(*allocTemp->Alloc)(*puVar7 * 8);
//           *unPackSizes = puVar5;
//           if (puVar5 == (ulonglong *)0x0) {
//             return -0x7ff8fff2;
//           }
//           puVar6 = (uchar *)(*allocTemp->Alloc)(*puVar7);
//           *digestsDefined = puVar6;
//           if (puVar6 == (uchar *)0x0) {
//             return -0x7ff8fff2;
//           }
//           puVar7 = (uint *)(*allocTemp->Alloc)(*puVar7 * 4);
//           *digests = puVar7;
//           if (puVar7 == (uint *)0x0) {
//             return -0x7ff8fff2;
//           }
//         }
//         local_20 = (int *)0x0;
//         if (numFolders != 0) {
//           local_28 = folders;
//           do {
//             uVar11 = local_28->NumUnPackStreams;
//             local_10 = 0;
//             local_c = 0;
//             if (uVar11 != 0) {
//               if (((iVar9 == 9) && (iVar12 == 0)) && (uVar13 = 1, 1 < uVar11)) {
//                 do {
//                   iVar4 = SzReadNumber((_CSzState *)unaff_EDI,unaff_ESI);
//                   if (iVar4 != 0) {
//                     return iVar4;
//                   }
//                   puVar5 = *unPackSizes;
//                   *(uint *)(puVar5 + local_2c) = local_8;
//                   local_2c = local_2c + 1;
//                   bVar14 = CARRY4(local_10,local_8);
//                   local_10 = local_10 + local_8;
//                   *(int *)((int)puVar5 + local_2c * 8 + -4) = local_4;
//                   local_c = local_c + local_4 + (uint)bVar14;
//                   uVar13 = uVar13 + 1;
//                 } while (uVar13 < uVar11);
//               }
//               uVar15 = SzFolderGetUnPackSize(unaff_EDI);
//               lVar1 = uVar15 - CONCAT44(local_c,local_10);
//               puVar5 = *unPackSizes;
//               *(int *)(puVar5 + local_2c) = (int)lVar1;
//               *(int *)((int)puVar5 + local_2c * 8 + 4) = (int)((ulonglong)lVar1 >> 0x20);
//               local_2c = local_2c + 1;
//               iVar9 = local_18;
//               iVar12 = local_14;
//             }
//             local_28 = local_28 + 1;
//             local_20 = (int *)((int)local_20 + 1);
//           } while (local_20 < numFolders);
//         }
//         if (((iVar9 == 9) && (iVar12 == 0)) &&
//            (iVar4 = SzReadNumber((_CSzState *)unaff_EDI,unaff_ESI), iVar4 != 0)) {
//           return iVar4;
//         }
//         uVar11 = 0;
//         if (*numUnPackStreams != 0) {
//           uVar13 = *numUnPackStreams;
//           puVar7 = *digests;
//           puVar6 = *digestsDefined;
//           do {
//             puVar6[uVar11] = '\0';
//             *puVar7 = 0;
//             uVar11 = uVar11 + 1;
//             puVar7 = puVar7 + 1;
//           } while (uVar11 < uVar13);
//         }
//         if (numFolders != 0) {
//           piVar8 = &folders->UnPackCRCDefined;
//           uVar11 = numFolders;
//           do {
//             if ((piVar8[2] != 1) || (*piVar8 == 0)) {
//               local_24 = local_24 + piVar8[2];
//             }
//             piVar8 = piVar8 + 10;
//             uVar11 = uVar11 - 1;
//           } while (uVar11 != 0);
//         }
//         iVar4 = 0;
//         do {
//           if ((local_18 == 10) && (local_14 == 0)) {
//             iVar12 = 0;
//             local_20 = (int *)0x0;
//             numUnPackStreams = (uint *)0x0;
//             iVar9 = SzReadHashDigests(sd,local_24,(uchar **)&local_20,&numUnPackStreams,unaff_EDI);
//             piVar8 = local_20;
//             p_Var2 = allocTemp;
//             piVar10 = local_20;
//             if ((iVar9 == 0) && (numFolders != 0)) {
//               piVar10 = &folders->UnPackCRCDefined;
//               local_28 = (_CFolder *)numFolders;
//               do {
//                 uVar11 = piVar10[2];
//                 if ((uVar11 == 1) && (*piVar10 != 0)) {
//                   (*digestsDefined)[iVar4] = '\x01';
//                   (*digests)[iVar4] = piVar10[1];
//                   iVar4 = iVar4 + 1;
//                 }
//                 else if (uVar11 != 0) {
//                   puVar6 = *digestsDefined;
//                   puVar7 = *digests + iVar4;
//                   do {
//                     puVar6[iVar4] = *(uchar *)((int)local_20 + iVar12);
//                     *puVar7 = numUnPackStreams[iVar12];
//                     iVar4 = iVar4 + 1;
//                     puVar7 = puVar7 + 1;
//                     iVar12 = iVar12 + 1;
//                     uVar11 = uVar11 - 1;
//                   } while (uVar11 != 0);
//                 }
//                 piVar10 = piVar10 + 10;
//                 local_28 = (_CFolder *)((int)local_28 - 1);
//               } while (local_28 != (_CFolder *)0x0);
//             }
//             local_20 = piVar10;
//             (*allocTemp->Free)(piVar8);
//             (*p_Var2->Free)(numUnPackStreams);
//             if (iVar9 != 0) {
//               return iVar9;
//             }
//           }
//           else {
//             if (local_18 == 0 && local_14 == 0) {
//               return 0;
//             }
//             iVar9 = SzReadNumber((_CSzState *)unaff_EDI,unaff_ESI);
//             if (iVar9 != 0) {
//               return iVar9;
//             }
//             if ((local_4 == 0) && (local_8 <= sd->Size)) {
//               sd->Data = sd->Data + local_8;
//               sd->Size = sd->Size - local_8;
//               iVar9 = 0;
//             }
//             else {
//               iVar9 = 6;
//             }
//             if (iVar9 != 0) {
//               return iVar9;
//             }
//           }
//           iVar9 = SzReadNumber((_CSzState *)unaff_EDI,unaff_ESI);
//           if (iVar9 != 0) {
//             return iVar9;
//           }
//         } while( true );
//       }
//       iVar4 = SzReadNumber((_CSzState *)unaff_EDI,unaff_ESI);
//       if (iVar4 != 0) {
//         return iVar4;
//       }
//       if ((local_1c == 0) && (local_20 <= (int *)sd->Size)) {
//         sd->Data = (uchar *)((int)local_20 + (int)sd->Data);
//         sd->Size = (int)sd->Size - (int)local_20;
//         iVar4 = 0;
//       }
//       else {
//         iVar4 = 6;
//       }
//       if (iVar4 != 0) {
//         return iVar4;
//       }
//     }
//     iVar4 = SzReadNumber((_CSzState *)unaff_EDI,unaff_ESI);
//     puVar7 = numUnPackStreams;
//     iVar9 = local_18;
//     iVar12 = local_14;
//   } while( true );
// }
// ABI / bounds, original assembly:
// 00478e70 SUB ESP,0x2c
// 00478e73 MOV EDX,dword ptr [ESP + 0x34]
// 00478e77 PUSH EBX
// 00478e78 MOV EBX,dword ptr [ESP + 0x34]
// 00478e7c XOR EAX,EAX
// 00478e7e PUSH EBP
// 00478e7f MOV EBP,dword ptr [ESP + 0x44]
// 004792f2 XOR EAX,EAX
// 004792f4 POP EDI
// 004792f5 POP ESI
// 004792f6 POP EBP
// 004792f7 POP EBX
// 004792f8 ADD ESP,0x2c
// 004792fb RET
// Incoming reference SzReadStreamsInfo:
// 00479877 PUSH EAX
// 00479878 MOV EAX,dword ptr [EDI + 0x10]
// 0047987b PUSH ECX
// 0047987c PUSH EDX
// 0047987d PUSH EAX
// 0047987e PUSH EBX
// 0047987f CALL 0x00478e70
// 00479884 ADD ESP,0x20
// 00479887 TEST EAX,EAX
// 00479889 JNZ 0x004798b1
// VC9 /GL /LTCG asmcmp 2026-10-22: 96.03%.
// FUNCTION: VC1 0x478e70
// SzReadSubStreamsInfo

// Ghidra VirtualCity.Unwrapped.exe 0x479300..004795fc.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadUnPackInfo(_CSzState *sd,uint *numFolders,_CFolder **folders,void *allocFunc,
//                     _ISzAlloc *allocTemp)
//
// {
//   uchar uVar1;
//   uchar *puVar2;
//   uint uVar3;
//   _CFolder *p_Var4;
//   undefined4 *puVar5;
//   int iVar6;
//   ulonglong *puVar7;
//   int iVar8;
//   ulonglong *unaff_ESI;
//   _CSzState *unaff_EDI;
//   uint uVar9;
//   uint *local_1c;
//   uchar *local_18;
//   uint local_14;
//   int local_10;
//   int local_c;
//   uint uStack_8;
//   int iStack_4;
//
//   uVar3 = SzWaitAttribute((_CSzState *)0xb,CONCAT44(unaff_ESI,unaff_EDI));
//   if ((uVar3 == 0) &&
//      (uVar3 = SzReadNumber32((_CSzState *)numFolders,(uint *)unaff_EDI), uVar3 == 0)) {
//     if (sd->Size == 0) {
//       return 6;
//     }
//     sd->Size = sd->Size - 1;
//     uVar1 = *sd->Data;
//     sd->Data = sd->Data + 1;
//     uVar3 = -(uint)(uVar1 != '\0') & 6;
//     if (uVar3 == 0) {
//       if (*numFolders == 0) {
//         *folders = (_CFolder *)0x0;
//       }
//       else {
//         p_Var4 = (_CFolder *)(*allocFunc)(*numFolders * 0x28);
//         *folders = p_Var4;
//         if (p_Var4 == (_CFolder *)0x0) {
//           return -0x7ff8fff2;
//         }
//       }
//       uVar3 = 0;
//       if (*numFolders != 0) {
//         iVar8 = 0;
//         do {
//           puVar5 = (undefined4 *)((int)&(*folders)->NumCoders + iVar8);
//           uVar3 = uVar3 + 1;
//           *puVar5 = 0;
//           puVar5[1] = 0;
//           puVar5[2] = 0;
//           puVar5[3] = 0;
//           puVar5[4] = 0;
//           puVar5[5] = 0;
//           puVar5[6] = 0;
//           puVar5[7] = 0;
//           puVar5[8] = 0;
//           puVar5[9] = 0;
//           iVar8 = iVar8 + 0x28;
//         } while (uVar3 < *numFolders);
//       }
//       local_1c = (uint *)0x0;
//       if (*numFolders != 0) {
//         iVar8 = 0;
//         do {
//           iVar6 = SzGetNextFolderItem((_CSzState *)((int)&(*folders)->NumCoders + iVar8),allocFunc,
//                                       unaff_EDI);
//           if (iVar6 != 0) {
//             return iVar6;
//           }
//           local_1c = (uint *)((int)local_1c + 1);
//           iVar8 = iVar8 + 0x28;
//         } while (local_1c < (uint *)*numFolders);
//       }
//       uVar3 = SzWaitAttribute((_CSzState *)0xc,CONCAT44(unaff_ESI,unaff_EDI));
//       if (uVar3 == 0) {
//         local_1c = (uint *)0x0;
//         if (*numFolders != 0) {
//           local_18 = (uchar *)0x0;
//           do {
//             uVar9 = 0;
//             p_Var4 = (_CFolder *)(local_18 + (int)&(*folders)->NumCoders);
//             uVar3 = SzFolderGetNumOutStreams(p_Var4);
//             local_14 = uVar3;
//             if (uVar3 == 0) {
//               p_Var4->UnPackSizes = (ulonglong *)0x0;
//             }
//             else {
//               puVar7 = (ulonglong *)(*allocFunc)(uVar3 * 8);
//               p_Var4->UnPackSizes = puVar7;
//               if (puVar7 == (ulonglong *)0x0) {
//                 return -0x7ff8fff2;
//               }
//             }
//             if (uVar3 != 0) {
//               do {
//                 iVar8 = SzReadNumber(unaff_EDI,unaff_ESI);
//                 if (iVar8 != 0) {
//                   return iVar8;
//                 }
//                 puVar7 = p_Var4->UnPackSizes;
//                 *(int *)(puVar7 + uVar9) = local_10;
//                 *(int *)((int)puVar7 + uVar9 * 8 + 4) = local_c;
//                 uVar9 = uVar9 + 1;
//               } while (uVar9 < local_14);
//             }
//             local_18 = local_18 + 0x28;
//             local_1c = (uint *)((int)local_1c + 1);
//           } while (local_1c < (uint *)*numFolders);
//         }
//         uVar3 = SzReadNumber(unaff_EDI,unaff_ESI);
//         if (uVar3 == 0) {
//           while( true ) {
//             if (local_10 == 0 && local_c == 0) {
//               return 0;
//             }
//             if ((local_10 == 10) && (local_c == 0)) {
//               local_18 = (uchar *)0x0;
//               local_1c = (uint *)0x0;
//               uVar3 = SzReadHashDigests(sd,*numFolders,&local_18,&local_1c,unaff_EDI);
//               puVar2 = local_18;
//               if ((uVar3 == 0) && (uVar9 = 0, *numFolders != 0)) {
//                 iVar8 = 0;
//                 do {
//                   p_Var4 = *folders;
//                   *(uint *)((int)&p_Var4->UnPackCRCDefined + iVar8) = (uint)local_18[uVar9];
//                   *(uint *)((int)&p_Var4->UnPackCRC + iVar8) = local_1c[uVar9];
//                   uVar9 = uVar9 + 1;
//                   iVar8 = iVar8 + 0x28;
//                 } while (uVar9 < *numFolders);
//               }
//               local_14 = uVar3;
//               (*allocTemp->Free)(local_1c);
//               (*allocTemp->Free)(puVar2);
//             }
//             else {
//               iVar8 = SzReadNumber(unaff_EDI,unaff_ESI);
//               if (iVar8 != 0) {
//                 return iVar8;
//               }
//               if ((iStack_4 == 0) && (uStack_8 <= sd->Size)) {
//                 sd->Data = sd->Data + uStack_8;
//                 sd->Size = sd->Size - uStack_8;
//                 uVar3 = 0;
//               }
//               else {
//                 uVar3 = 6;
//               }
//             }
//             if (uVar3 != 0) break;
//             iVar8 = SzReadNumber(unaff_EDI,unaff_ESI);
//             if (iVar8 != 0) {
//               return iVar8;
//             }
//           }
//           return uVar3;
//         }
//       }
//     }
//   }
//   return uVar3;
// }
// ABI / bounds, original assembly:
// 00479300 SUB ESP,0x20
// 00479303 PUSH EBX
// 00479304 MOV EBX,dword ptr [ESP + 0x28]
// 00479308 PUSH EBP
// 00479309 PUSH ESI
// 0047930a PUSH EDI
// 0047930b XOR EDI,EDI
// 004795f3 MOV EAX,EBX
// 004795f5 POP EDI
// 004795f6 POP ESI
// 004795f7 POP EBP
// 004795f8 POP EBX
// 004795f9 ADD ESP,0x20
// 004795fc RET
// Incoming reference SzReadStreamsInfo:
// 0047984d PUSH ECX
// 0047984e LEA EDX,[EDI + 0x14]
// 00479851 PUSH EDX
// 00479852 LEA EAX,[EDI + 0x10]
// 00479855 PUSH EAX
// 00479856 PUSH EBX
// 00479857 CALL 0x00479300
// 0047985c ADD ESP,0x14
// 0047985f JMP 0x00479887
// 00479861 MOV ECX,dword ptr [ESP + 0x2c]
// VC9 /GL /LTCG asmcmp 2026-10-22: 66.18%.
// FUNCTION: VC1 0x479300
// SzReadUnPackInfo

// Ghidra VirtualCity.Unwrapped.exe 0x479600..004797dd.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadPackInfo(_CSzState *param0,ulonglong *param1,uint *param2,ulonglong **param3,
//                   uchar **param4,uint **param5,void *param6)
//
// {
//   ulonglong uVar1;
//   int *in_EAX;
//   int iVar2;
//   uint uVar3;
//   ulonglong *puVar4;
//   _CSzState *in_ECX;
//   ulonglong *unaff_ESI;
//   _CSzState *unaff_EDI;
//   uchar *puVar5;
//   int local_10;
//   int local_c;
//   uint local_8;
//   int local_4;
//
//   iVar2 = SzReadNumber(unaff_EDI,unaff_ESI);
//   if (iVar2 == 0) {
//     *in_EAX = local_10;
//     in_EAX[1] = local_c;
//     iVar2 = SzReadNumber32(param0,(uint *)unaff_EDI);
//     if (iVar2 != 0) {
//       return iVar2;
//     }
//     iVar2 = SzWaitAttribute((_CSzState *)&DAT_00000009,CONCAT44(unaff_ESI,unaff_EDI));
//     if (iVar2 != 0) {
//       return iVar2;
//     }
//     if (param0->Data == (uchar *)0x0) {
//       *(undefined4 *)param1 = 0;
//     }
//     else {
//       iVar2 = (*(code *)param4)((int)param0->Data * 8);
//       *(int *)param1 = iVar2;
//       if (iVar2 == 0) {
//         return -0x7ff8fff2;
//       }
//     }
//     puVar5 = (uchar *)0x0;
//     if (param0->Data != (uchar *)0x0) {
//       do {
//         iVar2 = SzReadNumber(unaff_EDI,unaff_ESI);
//         if (iVar2 != 0) {
//           return iVar2;
//         }
//         uVar1 = *param1;
//         *(int *)((int)uVar1 + (int)puVar5 * 8) = local_10;
//         *(int *)((int)uVar1 + 4 + (int)puVar5 * 8) = local_c;
//         puVar5 = puVar5 + 1;
//       } while (puVar5 < param0->Data);
//     }
//     iVar2 = SzReadNumber(unaff_EDI,unaff_ESI);
//     if (iVar2 != 0) {
//       return iVar2;
//     }
//     while (local_10 != 0 || local_c != 0) {
//       if ((local_10 == 10) && (local_c == 0)) {
//         iVar2 = SzReadHashDigests(in_ECX,(uint)param0->Data,(uchar **)param2,(uint **)param3,
//                                   unaff_EDI);
//       }
//       else {
//         iVar2 = SzReadNumber(unaff_EDI,unaff_ESI);
//         if (iVar2 != 0) {
//           return iVar2;
//         }
//         if ((local_4 == 0) && (local_8 <= in_ECX->Size)) {
//           in_ECX->Data = in_ECX->Data + local_8;
//           in_ECX->Size = in_ECX->Size - local_8;
//           iVar2 = 0;
//         }
//         else {
//           iVar2 = 6;
//         }
//       }
//       if (iVar2 != 0) {
//         return iVar2;
//       }
//       iVar2 = SzReadNumber(unaff_EDI,unaff_ESI);
//       if (iVar2 != 0) {
//         return iVar2;
//       }
//     }
//     if (*param2 == 0) {
//       if (param0->Data == (uchar *)0x0) {
//         *param2 = 0;
//       }
//       else {
//         uVar3 = (*(code *)param4)(param0->Data);
//         *param2 = uVar3;
//         if (uVar3 == 0) {
//           return -0x7ff8fff2;
//         }
//       }
//       if (param0->Data == (uchar *)0x0) {
//         *param3 = (ulonglong *)0x0;
//       }
//       else {
//         puVar4 = (ulonglong *)(*(code *)param4)((int)param0->Data * 4);
//         *param3 = puVar4;
//         if (puVar4 == (ulonglong *)0x0) {
//           return -0x7ff8fff2;
//         }
//       }
//       puVar5 = (uchar *)0x0;
//       if (param0->Data != (uchar *)0x0) {
//         do {
//           puVar5[*param2] = '\0';
//           *(undefined4 *)((int)*param3 + (int)puVar5 * 4) = 0;
//           puVar5 = puVar5 + 1;
//         } while (puVar5 < param0->Data);
//       }
//     }
//     iVar2 = 0;
//   }
//   return iVar2;
// }
// ABI / bounds, original assembly:
// 00479600 SUB ESP,0x10
// 00479603 PUSH EBX
// 00479604 PUSH EBP
// 00479605 MOV EBP,dword ptr [ESP + 0x20]
// 00479609 PUSH ESI
// 0047960a PUSH EDI
// 0047960b MOV EBX,ECX
// 004797d4 XOR EAX,EAX
// 004797d6 POP EDI
// 004797d7 POP ESI
// 004797d8 POP EBP
// 004797d9 POP EBX
// 004797da ADD ESP,0x10
// 004797dd RET
// Incoming reference SzReadStreamsInfo:
// 00479832 PUSH EDX
// 00479833 LEA EAX,[EDI + 0x4]
// 00479836 PUSH EAX
// 00479837 MOV EAX,dword ptr [ESP + 0x2c]
// 0047983b PUSH EDI
// 0047983c MOV ECX,EBX
// 0047983e CALL 0x00479600
// 00479843 ADD ESP,0x14
// 00479846 JMP 0x00479887
// 00479848 MOV ECX,dword ptr [ESP + 0x30]
// VC9 /GL /LTCG asmcmp 2026-10-22: 92.26%.
// FUNCTION: VC1 0x479600
// SzReadPackInfo

// Ghidra VirtualCity.Unwrapped.exe 0x4797e0..004798b7.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadStreamsInfo(_CSzState *param0,ulonglong *param1,_CArchiveDatabase *param2,uint *param3,
//                      ulonglong **param4,uchar **param5,uint **param6,void *param7,_ISzAlloc *param8)
//
// {
//   _CSzState *in_EAX;
//   int iVar1;
//   ulonglong *unaff_EBP;
//   _CSzState *unaff_ESI;
//   _CSzState *unaff_EDI;
//   int local_8;
//   int local_4;
//
//   iVar1 = SzReadNumber(unaff_ESI,unaff_EBP);
//   if (iVar1 == 0) {
//     while (local_8 >> 0x1f == local_4) {
//       switch(local_8) {
//       case 0:
//         return 0;
//       default:
//         goto switchD_0047981f_caseD_1;
//       case 6:
//         iVar1 = SzReadPackInfo(unaff_EDI,(ulonglong *)&unaff_EDI->Size,(uint *)(unaff_EDI + 1),
//                                (ulonglong **)&unaff_EDI[1].Size,param5,(uint **)unaff_ESI,unaff_EBP)
//         ;
//         break;
//       case 7:
//         iVar1 = SzReadUnPackInfo(in_EAX,(uint *)(unaff_EDI + 2),(_CFolder **)&unaff_EDI[2].Size,
//                                  param5,(_ISzAlloc *)param6);
//         break;
//       case 8:
//         iVar1 = SzReadSubStreamsInfo
//                           (in_EAX,(uint)unaff_EDI[2].Data,(_CFolder *)unaff_EDI[2].Size,
//                            (uint *)param1,(ulonglong **)param2,(uchar **)param3,(uint **)param4,
//                            (_ISzAlloc *)param6);
//       }
//       if (iVar1 != 0) {
//         return iVar1;
//       }
//       iVar1 = SzReadNumber(unaff_ESI,unaff_EBP);
//       if (iVar1 != 0) {
//         return iVar1;
//       }
//     }
// switchD_0047981f_caseD_1:
//     iVar1 = -0x7fffbffb;
//   }
//   return iVar1;
// }
// ABI / bounds, original assembly:
// 004797e0 SUB ESP,0xc
// 004797e3 PUSH EBX
// 004797e4 PUSH EBP
// 004797e5 MOV EBP,dword ptr [ESP + 0x30]
// 004797e9 PUSH ESI
// 004797ea MOV EBX,EAX
// 004797ec LEA ESI,[ESP + 0x10]
// 004798ab RET
// 004798ac MOV EAX,0x80004005
// 004798b1 POP ESI
// 004798b2 POP EBP
// 004798b3 POP EBX
// 004798b4 ADD ESP,0xc
// 004798b7 RET
// Incoming reference SzReadAndDecodePackedStreams2:
// 004798f8 MOV EAX,dword ptr [ESP + 0x3c]
// 004798fc LEA ECX,[ESP + 0x24]
// 00479900 PUSH ECX
// 00479901 LEA EDX,[ESP + 0x30]
// 00479905 PUSH EDX
// 00479906 MOV dword ptr [ESP + 0x2c],0x0
// 0047990e CALL 0x004797e0
// 00479913 ADD ESP,0x1c
// 00479916 TEST EAX,EAX
// 00479918 JNZ 0x004799f4
// VC9 /GL /LTCG asmcmp 2026-10-22: 70.16%.
// FUNCTION: VC1 0x4797e0
// SzReadStreamsInfo

// Ghidra VirtualCity.Unwrapped.exe 0x4798e0..004799fb.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadAndDecodePackedStreams2
//               (_ISzInStream *inStream,_CSzState *sd,_CSzByteBuffer *outBuffer,ulonglong param3,
//               _CArchiveDatabase *param4,ulonglong **param5,uchar **param6,uint **param7,
//               _ISzAlloc *param8)
//
// {
//   _CFolder *folder;
//   code *pcVar1;
//   uint **in_EAX;
//   int iVar2;
//   void *param0;
//   uint uVar3;
//   uchar *puVar4;
//   ulonglong **in_ECX;
//   uint *in_EDX;
//   _ISzAlloc *unaff_EBX;
//   uint unaff_EBP;
//   _ISzAlloc *unaff_ESI;
//   _CFolder *unaff_EDI;
//   bool bVar5;
//   ulonglong uVar6;
//   uint in_stack_00000010;
//   undefined4 local_10;
//   undefined4 local_c;
//   _CSzState local_8;
//
//   local_10 = 0;
//   iVar2 = SzReadStreamsInfo(&local_8,(ulonglong *)&local_10,param4,in_EDX,in_ECX,(uchar **)*in_EAX,
//                             in_EAX,unaff_EDI,unaff_ESI);
//   if (iVar2 == 0) {
//     bVar5 = CARRY4((uint)local_8.Data,in_stack_00000010);
//     local_8.Data = local_8.Data + in_stack_00000010;
//     local_8.Size = local_8.Size + (int)param3 + (uint)bVar5;
//     if (*(int *)(param3._4_4_ + 0x10) != 1) {
//       return 6;
//     }
//     folder = *(_CFolder **)(param3._4_4_ + 0x14);
//     uVar6 = SzFolderGetUnPackSize(unaff_EDI);
//     local_c = (undefined4)(uVar6 >> 0x20);
//     param0 = (void *)uVar6;
//     iVar2 = (*inStream->Seek)(inStream,local_8.Data,local_8.Size);
//     if (iVar2 == 0) {
//       pcVar1 = (code *)*in_EAX;
//       outBuffer->Capacity = (uint)param0;
//       if (param0 == (void *)0x0) {
//         outBuffer->Items = (uchar *)0x0;
//       }
//       else {
//         puVar4 = (uchar *)(*pcVar1)(param0);
//         outBuffer->Items = puVar4;
//         if (puVar4 == (uchar *)0x0) {
//           return -0x7ff8fff2;
//         }
//       }
//       iVar2 = SzDecode(*(ulonglong **)(param3._4_4_ + 4),folder,inStream,
//                        CONCAT44(unaff_EDI,local_8.Size),(uchar *)unaff_ESI,unaff_EBP,unaff_EBX);
//       if (iVar2 == 0) {
//         if (folder->UnPackCRCDefined != 0) {
//           uVar3 = CrcCalc(param0,(uint)outBuffer->Items);
//           if (uVar3 != folder->UnPackCRC) {
//             return -0x7fffbffb;
//           }
//         }
//         iVar2 = 0;
//       }
//     }
//   }
//   return iVar2;
// }
// ABI / bounds, original assembly:
// 004798e0 SUB ESP,0x10
// 004798e3 PUSH EBX
// 004798e4 PUSH EBP
// 004798e5 PUSH ESI
// 004798e6 PUSH EDI
// 004798e7 MOV EDI,dword ptr [ESP + 0x38]
// 004798eb MOV EBX,EAX
// 004799f2 XOR EAX,EAX
// 004799f4 POP EDI
// 004799f5 POP ESI
// 004799f6 POP EBP
// 004799f7 POP EBX
// 004799f8 ADD ESP,0x10
// 004799fb RET
// Incoming reference SzReadAndDecodePackedStreams:
// 00479e52 PUSH ECX
// 00479e53 PUSH EDX
// 00479e54 PUSH EAX
// 00479e55 MOV EAX,ESI
// 00479e57 LEA ECX,[ESP + 0x28]
// 00479e5b LEA EDX,[ESP + 0x24]
// 00479e5f CALL 0x004798e0
// 00479e64 MOV ECX,dword ptr [ESI + 0x4]
// 00479e67 ADD ESP,0x1c
// 00479e6a PUSH ECX
// VC9 /GL /LTCG asmcmp 2026-10-22: 80.53%.
// FUNCTION: VC1 0x4798e0
// SzReadAndDecodePackedStreams2

// Ghidra VirtualCity.Unwrapped.exe 0x479a00..00479ddd.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadHeader2(_CSzState *sd,_CArchiveDatabaseEx *db,ulonglong **unPackSizes,
//                  uchar **digestsDefined,uint **digests,uchar **emptyStreamVector,
//                  uchar **emptyFileVector,uchar **lwtVector,_ISzAlloc *allocMain,_ISzAlloc *allocTemp
//                  )
//
// {
//   int *piVar1;
//   uchar uVar2;
//   uint uVar3;
//   ulonglong *puVar4;
//   ulonglong uVar5;
//   undefined4 uVar6;
//   int iVar7;
//   uchar *puVar8;
//   int iVar9;
//   uchar *puVar10;
//   void *unaff_EBP;
//   _ISzAlloc *unaff_ESI;
//   uchar *puVar11;
//   uint *puVar12;
//   _CSzState *unaff_EDI;
//   bool bVar13;
//   undefined1 local_1c [8];
//   int local_14;
//   _CSzState *local_10;
//   int local_c;
//   int local_8;
//   int local_4;
//
//   local_1c._4_4_ = 0;
//   local_1c._0_4_ = (uchar *)0x0;
//   local_14 = 0;
//   iVar7 = SzReadNumber(unaff_EDI,(ulonglong *)unaff_ESI);
//   if (iVar7 != 0) {
//     return iVar7;
//   }
//   if ((local_10 == (_CSzState *)0x2) && (local_c == 0)) {
//     iVar7 = SzReadArchiveProperties(unaff_EDI);
//     if (iVar7 != 0) {
//       return iVar7;
//     }
//     iVar7 = SzReadNumber(unaff_EDI,(ulonglong *)unaff_ESI);
//     if (iVar7 != 0) {
//       return iVar7;
//     }
//   }
//   if ((local_10 == (_CSzState *)&DAT_00000004) && (local_c == 0)) {
//     puVar4 = &(db->ArchiveInfo).DataStartPosition;
//     iVar7 = SzReadStreamsInfo((_CSzState *)puVar4,(ulonglong *)(local_1c + 4),
//                               (_CArchiveDatabase *)unPackSizes,(uint *)digestsDefined,
//                               (ulonglong **)digests,allocMain->Alloc,&allocTemp->Alloc,unaff_EDI,
//                               unaff_ESI);
//     if (iVar7 != 0) {
//       return iVar7;
//     }
//     uVar3 = (uint)(db->ArchiveInfo).StartPositionAfterHeader;
//     uVar5 = *puVar4;
//     *(uint *)puVar4 = (int)*puVar4 + uVar3;
//     piVar1 = (int *)((int)&(db->ArchiveInfo).DataStartPosition + 4);
//     *piVar1 = *piVar1 + *(int *)((int)&(db->ArchiveInfo).StartPositionAfterHeader + 4) +
//               (uint)CARRY4((uint)uVar5,uVar3);
//     iVar7 = SzReadNumber(unaff_EDI,(ulonglong *)unaff_ESI);
//     if (iVar7 != 0) {
//       return iVar7;
//     }
//   }
//   if (local_10 == (_CSzState *)0x0 && local_c == 0) {
//     return 0;
//   }
//   if ((local_10 != (_CSzState *)&DAT_00000005) || (local_c != 0)) {
//     return 6;
//   }
//   iVar7 = SzReadNumber32((_CSzState *)local_1c,(uint *)unaff_EDI);
//   puVar11 = (uchar *)local_1c._0_4_;
//   if (iVar7 == 0) {
//     (db->Database).NumFiles = local_1c._0_4_;
//     if ((uchar *)local_1c._0_4_ == (uchar *)0x0) {
//       local_1c._4_4_ = (_CFileItem *)0x0;
//     }
//     else {
//       local_1c._4_4_ = (*allocMain->Alloc)(local_1c._0_4_ << 5);
//       if ((_CFileItem *)local_1c._4_4_ == (_CFileItem *)0x0) {
//         return -0x7ff8fff2;
//       }
//     }
//     (db->Database).Files = (_CFileItem *)local_1c._4_4_;
//     if (puVar11 != (uchar *)0x0) {
//       puVar8 = (uchar *)(local_1c._4_4_ + 0x19);
//       do {
//         puVar8[-1] = '\0';
//         *puVar8 = '\x01';
//         puVar8[1] = '\0';
//         puVar8[2] = '\0';
//         puVar8[3] = '\0';
//         *(char **)(puVar8 + -5) = (char *)0x0;
//         puVar8 = puVar8 + 0x20;
//         puVar11 = puVar11 + -1;
//       } while (puVar11 != (uchar *)0x0);
//     }
//     iVar7 = SzReadNumber(unaff_EDI,(ulonglong *)unaff_ESI);
//     if (iVar7 == 0) {
//       while (iVar7 = local_8, local_8 != 0 || local_4 != 0) {
//         iVar9 = SzReadNumber(unaff_EDI,(ulonglong *)unaff_ESI);
//         uVar6 = local_1c._0_4_;
//         if (iVar9 != 0) {
//           return iVar9;
//         }
//         if (iVar7 >> 0x1f != local_4) {
// LAB_00479cfa:
//           iVar7 = SzSkeepDataSize(local_10,CONCAT44(unaff_ESI,unaff_EDI));
//           goto LAB_00479d04;
//         }
//         switch(iVar7) {
//         case 0xe:
//           iVar7 = SzReadBoolVector((_CSzState *)emptyStreamVector,(uint)allocTemp->Alloc,
//                                    &unaff_EDI->Data,unaff_ESI);
//           if (iVar7 != 0) {
//             return iVar7;
//           }
//           local_14 = 0;
//           if ((uchar *)uVar6 != (uchar *)0x0) {
//             puVar11 = (uchar *)0x0;
//             do {
//               if ((*emptyStreamVector)[(int)puVar11] != '\0') {
//                 local_14 = local_14 + 1;
//               }
//               puVar11 = puVar11 + 1;
//             } while (puVar11 < (uint)local_1c._0_4_);
//           }
//           break;
//         case 0xf:
//           iVar7 = SzReadBoolVector((_CSzState *)emptyFileVector,(uint)allocTemp->Alloc,
//                                    &unaff_EDI->Data,unaff_ESI);
//           goto LAB_00479d04;
//         default:
//           goto LAB_00479cfa;
//         case 0x11:
//           iVar7 = SzReadSwitch(unaff_EDI);
//           if (iVar7 != 0) {
//             return iVar7;
//           }
//           iVar7 = SzReadFileNames(sd,local_1c._0_4_,allocMain->Alloc,unaff_EDI);
// LAB_00479d04:
//           if (iVar7 != 0) {
//             return iVar7;
//           }
//           break;
//         case 0x14:
//           iVar7 = SzReadBoolVector2(sd,(uint)unaff_EDI,&unaff_ESI->Alloc,unaff_EBP);
//           if (iVar7 != 0) {
//             return iVar7;
//           }
//           iVar7 = SzReadSwitch(unaff_EDI);
//           if (iVar7 != 0) {
//             return iVar7;
//           }
//           puVar11 = (uchar *)0x0;
//           if ((uchar *)uVar6 != (uchar *)0x0) {
//             puVar12 = &((_CArchiveFileTime *)local_1c._4_4_)->High;
//             do {
//               uVar2 = puVar11[(int)*lwtVector];
//               *(uchar *)(puVar12 + 6) = uVar2;
//               *puVar12 = 0;
//               ((_CArchiveFileTime *)(puVar12 + -1))->Low = 0;
//               if (uVar2 != '\0') {
//                 iVar7 = SzReadUInt32(unaff_EDI,(uint *)unaff_ESI);
//                 if (iVar7 != 0) {
//                   return iVar7;
//                 }
//                 iVar7 = SzReadUInt32(unaff_EDI,(uint *)unaff_ESI);
//                 if (iVar7 != 0) {
//                   return iVar7;
//                 }
//               }
//               puVar11 = puVar11 + 1;
//               puVar12 = puVar12 + 8;
//             } while (puVar11 < (uint)local_1c._0_4_);
//           }
//         }
//         iVar7 = SzReadNumber(unaff_EDI,(ulonglong *)unaff_ESI);
//         if (iVar7 != 0) {
//           return iVar7;
//         }
//       }
//       iVar7 = 0;
//       puVar11 = (uchar *)0x0;
//       local_14 = 0;
//       if ((uchar *)local_1c._0_4_ != (uchar *)0x0) {
//         puVar8 = *emptyStreamVector;
//         puVar10 = (uchar *)(local_1c._4_4_ + 0x19);
//         do {
//           puVar10[2] = '\0';
//           if (puVar8 == (uchar *)0x0) {
//             *puVar10 = '\x01';
// LAB_00479d5e:
//             puVar4 = *unPackSizes;
//             puVar10[1] = '\0';
//             *(int *)(puVar10 + -0x11) = (int)puVar4[iVar7];
//             *(undefined4 *)(puVar10 + -0xd) = *(undefined4 *)((int)puVar4 + iVar7 * 8 + 4);
//             *(uint *)(puVar10 + -9) = (*digests)[iVar7];
//             puVar10[-1] = (*digestsDefined)[iVar7];
//             iVar7 = iVar7 + 1;
//           }
//           else {
//             bVar13 = puVar8[(int)puVar11] == '\0';
//             *puVar10 = bVar13;
//             if (bVar13) goto LAB_00479d5e;
//             if (*emptyFileVector == (uchar *)0x0) {
//               puVar10[1] = '\x01';
//             }
//             else {
//               puVar10[1] = (*emptyFileVector)[local_14] == '\0';
//             }
//             local_14 = local_14 + 1;
//             *(undefined4 *)(puVar10 + -0x11) = 0;
//             *(undefined4 *)(puVar10 + -0xd) = 0;
//             puVar10[-1] = '\0';
//           }
//           puVar11 = puVar11 + 1;
//           puVar10 = puVar10 + 0x20;
//         } while (puVar11 < (uint)local_1c._0_4_);
//       }
//       iVar7 = SzArDbExFill(allocMain->Alloc,unaff_EDI);
//     }
//   }
//   return iVar7;
// }
// ABI / bounds, original assembly:
// 00479a00 SUB ESP,0x1c
// 00479a03 PUSH EBX
// 00479a04 PUSH EBP
// 00479a05 MOV EBP,dword ptr [ESP + 0x28]
// 00479a09 XOR EAX,EAX
// 00479a0b PUSH ESI
// 00479a0c PUSH EDI
// 00479dd3 ADD ESP,0x4
// 00479dd6 POP EDI
// 00479dd7 POP ESI
// 00479dd8 POP EBP
// 00479dd9 POP EBX
// 00479dda ADD ESP,0x1c
// 00479ddd RET
// Incoming reference SzReadHeader:
// 00479ef0 PUSH EDX
// 00479ef1 MOV EDX,dword ptr [ESP + 0x40]
// 00479ef5 LEA EAX,[ESP + 0x24]
// 00479ef9 PUSH EAX
// 00479efa PUSH ECX
// 00479efb PUSH EDX
// 00479efc CALL 0x00479a00
// 00479f01 MOV ECX,dword ptr [ESI + 0x4]
// 00479f04 MOV EDI,EAX
// 00479f06 MOV EAX,dword ptr [ESP + 0x30]
// VC9 /GL /LTCG asmcmp 2026-10-22: 96.25%.
// FUNCTION: VC1 0x479a00
// SzReadHeader2

// Ghidra VirtualCity.Unwrapped.exe 0x479e00..00479ea4.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadAndDecodePackedStreams
//               (_ISzInStream *inStream,_CSzState *sd,_CSzByteBuffer *outBuffer,ulonglong param3,
//               _ISzAlloc *param4)
//
// {
//   int iVar1;
//   int unaff_ESI;
//   ulonglong **unaff_EDI;
//   uint uVar2;
//   ulonglong *puVar3;
//   uchar *puVar4;
//   _CArchiveDatabase local_20;
//
//   uVar2 = 0;
//   puVar3 = (ulonglong *)0x0;
//   puVar4 = (uchar *)0x0;
//   local_20.NumPackStreams = 0;
//   local_20.PackSizes = (ulonglong *)0x0;
//   local_20.PackCRCsDefined = (uchar *)0x0;
//   local_20.PackCRCs = (uint *)0x0;
//   local_20.NumFolders = 0;
//   local_20.Folders = (_CFolder *)0x0;
//   local_20.NumFiles = 0;
//   local_20.Files = (_CFileItem *)0x0;
//   iVar1 = SzReadAndDecodePackedStreams2
//                     (inStream,sd,outBuffer,CONCAT44(&local_20,(undefined4)param3),
//                      (_CArchiveDatabase *)&stack0xffffffd4,unaff_EDI,(uchar **)0x0,(uint **)0x0,
//                      (_ISzAlloc *)0x0);
//   SzArchiveDatabaseFree(&local_20,*(void **)(unaff_ESI + 4));
//   (**(code **)(unaff_ESI + 4))(uVar2);
//   (**(code **)(unaff_ESI + 4))(puVar3);
//   (**(code **)(unaff_ESI + 4))(puVar4);
//   return iVar1;
// }
// ABI / bounds, original assembly:
// 00479e00 SUB ESP,0x2c
// 00479e03 MOV EDX,dword ptr [ESP + 0x40]
// 00479e07 XOR EAX,EAX
// 00479e09 PUSH EDI
// 00479e0a MOV dword ptr [ESP + 0x4],EAX
// 00479e0e MOV dword ptr [ESP + 0x8],EAX
// 00479e12 MOV dword ptr [ESP + 0xc],EAX
// 00479e98 PUSH ECX
// 00479e99 CALL EDX
// 00479e9b ADD ESP,0x4
// 00479e9e MOV EAX,EDI
// 00479ea0 POP EDI
// 00479ea1 ADD ESP,0x2c
// 00479ea4 RET
// Incoming reference SzArchiveOpen2:
// 0047a151 LEA EAX,[ESP + 0x18]
// 0047a155 PUSH EAX
// 0047a156 LEA ECX,[ESP + 0x2c]
// 0047a15a PUSH ECX
// 0047a15b PUSH EDX
// 0047a15c MOV ESI,EDI
// 0047a15e CALL 0x00479e00
// 0047a163 MOV EBX,EAX
// 0047a165 ADD ESP,0x14
// 0047a168 TEST EBX,EBX
// VC9 /GL /LTCG asmcmp 2026-10-22: 85.19%.
// FUNCTION: VC1 0x479e00
// SzReadAndDecodePackedStreams

// Ghidra VirtualCity.Unwrapped.exe 0x479eb0..00479f57.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzReadHeader(_CSzState *sd,_CArchiveDatabaseEx *db,_ISzAlloc *allocMain,_ISzAlloc *param3)
//
// {
//   int iVar1;
//   _ISzAlloc *unaff_ESI;
//   ulonglong *local_18;
//   uchar *local_14;
//   uint *local_10;
//   uchar *local_c;
//   uchar *local_8;
//   uchar *local_4;
//
//   local_18 = (ulonglong *)0x0;
//   local_14 = (uchar *)0x0;
//   local_10 = (uint *)0x0;
//   local_c = (uchar *)0x0;
//   local_8 = (uchar *)0x0;
//   local_4 = (uchar *)0x0;
//   iVar1 = SzReadHeader2(sd,db,&local_18,&local_14,&local_10,&local_c,&local_8,&local_4,allocMain,
//                         unaff_ESI);
//   (*unaff_ESI->Free)(local_18);
//   (*unaff_ESI->Free)(local_14);
//   (*unaff_ESI->Free)(local_10);
//   (*unaff_ESI->Free)(local_c);
//   (*unaff_ESI->Free)(local_8);
//   (*unaff_ESI->Free)(local_4);
//   return iVar1;
// }
// ABI / bounds, original assembly:
// 00479eb0 SUB ESP,0x1c
// 00479eb3 XOR EAX,EAX
// 00479eb5 PUSH EDI
// 00479eb6 PUSH ESI
// 00479eb7 MOV dword ptr [ESP + 0xc],EAX
// 00479ebb MOV dword ptr [ESP + 0x10],EAX
// 00479ebf MOV dword ptr [ESP + 0x14],EAX
// 00479f4b PUSH ECX
// 00479f4c CALL EDX
// 00479f4e ADD ESP,0x4
// 00479f51 MOV EAX,EDI
// 00479f53 POP EDI
// 00479f54 ADD ESP,0x1c
// 00479f57 RET
// Incoming reference SzArchiveOpen2:
// 0047a1b0 MOV EDX,dword ptr [EBP + 0xc]
// 0047a1b3 PUSH ECX
// 0047a1b4 PUSH EDX
// 0047a1b5 LEA EAX,[ESP + 0x28]
// 0047a1b9 PUSH EAX
// 0047a1ba MOV ESI,EDI
// 0047a1bc CALL 0x00479eb0
// 0047a1c1 ADD ESP,0xc
// 0047a1c4 MOV EBX,EAX
// 0047a1c6 JMP 0x0047a1df
// VC9 /GL /LTCG asmcmp 2026-10-22: 60.87%.
// FUNCTION: VC1 0x479eb0
// SzReadHeader

// Ghidra VirtualCity.Unwrapped.exe 0x479f60..0047a1f7.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzArchiveOpen2(_ISzInStream *param0,_CArchiveDatabaseEx *param1,_ISzAlloc *param2,
//                   _ISzAlloc *param3)
//
// {
//   uchar *param1_00;
//   int iVar1;
//   uchar *puVar2;
//   _ISzAlloc *unaff_ESI;
//   _ISzAlloc *unaff_EDI;
//   char local_31;
//   _CSzByteBuffer local_30;
//   uint local_28;
//   uchar *puStack_24;
//   _CSzState local_20;
//   undefined1 *local_18;
//   int local_14;
//   uint local_10;
//   int local_c;
//
//   local_30.Capacity = 0;
//   iVar1 = SafeReadDirect(param0,(uchar *)unaff_EDI,(uint)unaff_ESI);
//   if (iVar1 == 0) {
//     iVar1 = TestSignatureCandidate((uchar *)unaff_EDI);
//     if (iVar1 == 0) {
//       return 6;
//     }
//     iVar1 = SafeReadDirect(param0,(uchar *)unaff_EDI,(uint)unaff_ESI);
//     if (iVar1 == 0) {
//       if (local_31 != '\0') {
//         return 6;
//       }
//       iVar1 = SafeReadDirect(param0,(uchar *)unaff_EDI,(uint)unaff_ESI);
//       if (iVar1 == 0) {
//         iVar1 = SafeReadDirectUInt32(param0,(uint *)unaff_EDI,(uint *)unaff_ESI);
//         if (iVar1 == 0) {
//           local_30.Capacity = 0xffffffff;
//           iVar1 = SafeReadDirectUInt64(param0,(ulonglong *)&local_30,(uint *)unaff_EDI);
//           if (((iVar1 == 0) &&
//               (iVar1 = SafeReadDirectUInt64(param0,(ulonglong *)&local_30,(uint *)unaff_EDI),
//               iVar1 == 0)) &&
//              (iVar1 = SafeReadDirectUInt32(param0,(uint *)unaff_EDI,(uint *)unaff_ESI), iVar1 == 0))
//           {
//             *(undefined4 *)&(param1->ArchiveInfo).StartPositionAfterHeader = 0x20;
//             *(undefined4 *)((int)&(param1->ArchiveInfo).StartPositionAfterHeader + 4) = 0;
//             if (~local_30.Capacity != local_28) {
//               return 6;
//             }
//             if (local_18 == (undefined1 *)0x0 && local_14 == 0) {
//               return 0;
//             }
//             iVar1 = (*param0->Seek)(param0,local_10 + 0x20,local_c + (uint)(0xffffffdf < local_10));
//             if (iVar1 == 0) {
//               iVar1 = SzByteBufferCreate(param3->Alloc,(uint)unaff_EDI,unaff_ESI);
//               param1_00 = puStack_24;
//               if (iVar1 == 0) {
//                 return -0x7ff8fff2;
//               }
//               iVar1 = SafeReadDirect(param0,(uchar *)unaff_EDI,(uint)unaff_ESI);
//               if (iVar1 == 0) {
//                 iVar1 = 6;
//                 puVar2 = (uchar *)CrcCalc(local_18,(uint)param1_00);
//                 if (puVar2 == local_20.Data) {
//                   local_20.Data = param1_00;
//                   local_20.Size = local_28;
//                   iVar1 = SzReadNumber((_CSzState *)unaff_EDI,(ulonglong *)unaff_ESI);
//                   while (iVar1 == 0) {
//                     if ((local_18 == &DAT_00000001) && (local_14 == 0)) {
//                       iVar1 = SzReadHeader(&local_20,param1,param2,unaff_EDI);
//                       break;
//                     }
//                     if ((local_18 != (undefined1 *)0x17) || (local_14 != 0)) {
//                       iVar1 = 6;
//                       break;
//                     }
//                     iVar1 = SzReadAndDecodePackedStreams
//                                       (param0,&local_20,&local_30,
//                                        CONCAT44(unaff_EDI,
//                                                 *(undefined4 *)
//                                                  ((int)&(param1->ArchiveInfo).
//                                                         StartPositionAfterHeader + 4)),unaff_ESI);
//                     if (iVar1 != 0) {
//                       SzByteBufferFree(param3->Free,unaff_EDI);
//                       break;
//                     }
//                     SzByteBufferFree(param3->Free,unaff_EDI);
//                     puStack_24 = local_30.Items;
//                     local_28 = local_30.Capacity;
//                     local_20.Data = local_30.Items;
//                     local_20.Size = local_30.Capacity;
//                     iVar1 = SzReadNumber((_CSzState *)unaff_EDI,(ulonglong *)unaff_ESI);
//                   }
//                 }
//               }
//               SzByteBufferFree(param3->Free,unaff_EDI);
//             }
//           }
//         }
//       }
//     }
//   }
//   return iVar1;
// }
// ABI / bounds, original assembly:
// 00479f60 PUSH EBP
// 00479f61 MOV EBP,ESP
// 00479f63 AND ESP,0xfffffff8
// 00479f66 SUB ESP,0x2c
// 00479f69 PUSH EBX
// 00479f6a PUSH ESI
// 00479f6b MOV ESI,dword ptr [EBP + 0x8]
// 0047a1ef MOV EAX,EBX
// 0047a1f1 POP EDI
// 0047a1f2 POP ESI
// 0047a1f3 POP EBX
// 0047a1f4 MOV ESP,EBP
// 0047a1f6 POP EBP
// 0047a1f7 RET
// Incoming reference KdConstructNamed:
// 0047cc39 MOV dword ptr [ESI + 0x18],EDI
// 0047cc3c MOV dword ptr [ESI + 0x1c],EDI
// 0047cc3f MOV dword ptr [ESI + 0x30],EDI
// 0047cc42 MOV dword ptr [ESI + 0x34],EDI
// 0047cc45 MOV dword ptr [ESI + 0x38],EDI
// 0047cc48 MOV dword ptr [ESI + 0x3c],EDI
// 0047cc4b CALL 0x00479f60
// 0047cc50 ADD ESP,0x10
// 0047cc53 MOV dword ptr [ESP + 0x14],EAX
// 0047cc57 CMP EAX,EDI
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x479f60
// SzArchiveOpen2

// Ghidra VirtualCity.Unwrapped.exe 0x47a200..0x47a228.
// Upstream 7zIn.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzArchiveOpen(_ISzInStream *param0,_CArchiveDatabaseEx *param1,_ISzAlloc *param2,
//                  _ISzAlloc *param3)
//
// {
//   _ISzAlloc *in_EAX;
//   int iVar1;
//   _ISzInStream *in_ECX;
//   _ISzAlloc *in_EDX;
//   void *unaff_ESI;
//   _CArchiveDatabaseEx *unaff_EDI;
//
//   iVar1 = SzArchiveOpen2(in_ECX,(_CArchiveDatabaseEx *)param0,in_EDX,in_EAX);
//   if (iVar1 != 0) {
//     SzArDbExFree(unaff_EDI,unaff_ESI);
//   }
//   return iVar1;
// }
// ABI / bounds, original assembly:
// 0x47a200 push ebx
// 0x47a201 push esi
// 0x47a202 mov esi, dword ptr [esp + 0xc]
// 0x47a206 push edi
// 0x47a207 push eax
// 0x47a208 mov edi, edx
// 0x47a20a push edi
// 0x47a21b mov edi, dword ptr [edi + 4]
// 0x47a21e call 0x478e20
// 0x47a223 mov eax, ebx
// 0x47a225 pop edi
// 0x47a226 pop esi
// 0x47a227 pop ebx
// 0x47a228 ret
// Incoming reference none: retained uncalled body; ABI from assembly:
// VC9 /GL /LTCG asmcmp 2026-10-22: 100.00%.
// FUNCTION: VC1 0x47a200
// SzArchiveOpen

// Ghidra VirtualCity.Unwrapped.exe 0x4b70f0..004b70f0.
// Upstream 7zAlloc.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// void SzFreeTemp(void *param0)
//
// {
//   free(param0);
//   return;
// }
// ABI / bounds, original assembly:
// 004b70f0 JMP 0x0042f5de
// 004b70f0 JMP 0x0042f5de
// Incoming reference KdConstructNamed:
// 0047cbe1 CALL 0x00487140
// 0047cbe6 XOR EDI,EDI
// 0047cbe8 ADD ESP,0xc
// 0047cbeb LEA EBP,[EBX + 0xc]
// 0047cbee MOV dword ptr [EBX + 0x8],EAX
// 0047cbf1 MOV dword ptr [EBP],0x4b7100
// 0047cbf8 MOV dword ptr [EBX + 0x10],0x4b70f0
// 0047cbff MOV dword ptr [EBX],0x47c6f0
// 0047cc05 MOV dword ptr [EBX + 0x4],0x47c6a0
// 0047cc0c CMP EAX,EDI
// VC9 /GL /LTCG asmcmp 2026-10-22: 0.00%.
// FUNCTION: VC1 0x4b70f0 FOLDED
// SzFree

// Ghidra VirtualCity.Unwrapped.exe 0x4b70f0..004b70f0.
// Upstream 7zAlloc.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// void SzFreeTemp(void *param0)
//
// {
//   free(param0);
//   return;
// }
// ABI / bounds, original assembly:
// 004b70f0 JMP 0x0042f5de
// 004b70f0 JMP 0x0042f5de
// Incoming reference KdConstructNamed:
// 0047cbe1 CALL 0x00487140
// 0047cbe6 XOR EDI,EDI
// 0047cbe8 ADD ESP,0xc
// 0047cbeb LEA EBP,[EBX + 0xc]
// 0047cbee MOV dword ptr [EBX + 0x8],EAX
// 0047cbf1 MOV dword ptr [EBP],0x4b7100
// 0047cbf8 MOV dword ptr [EBX + 0x10],0x4b70f0
// 0047cbff MOV dword ptr [EBX],0x47c6f0
// 0047cc05 MOV dword ptr [EBX + 0x4],0x47c6a0
// 0047cc0c CMP EAX,EDI
// VC9 /GL /LTCG asmcmp 2026-10-22: 0.00%.
// FUNCTION: VC1 0x4b70f0 FOLDED
// SzFreeTemp

// Ghidra VirtualCity.Unwrapped.exe 0x4b7100..004b710d.
// Upstream 7zAlloc.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// void * SzAllocTemp(uint param0)
//
// {
//   void *pvVar1;
//
//   if (param0 == 0) {
//     return (void *)0x0;
//   }
//   pvVar1 = malloc(param0);
//   return pvVar1;
// }
// ABI / bounds, original assembly:
// 004b7100 MOV EAX,dword ptr [ESP + 0x4]
// 004b7104 TEST EAX,EAX
// 004b7106 JNZ 0x004b7109
// 004b7108 RET
// 004b7109 MOV dword ptr [ESP + 0x4],EAX
// 004b710d JMP 0x0042f6bb
// 004b7100 MOV EAX,dword ptr [ESP + 0x4]
// 004b7104 TEST EAX,EAX
// 004b7106 JNZ 0x004b7109
// 004b7108 RET
// 004b7109 MOV dword ptr [ESP + 0x4],EAX
// 004b710d JMP 0x0042f6bb
// Incoming reference KdConstructNamed:
// 0047cbe0 PUSH EAX
// 0047cbe1 CALL 0x00487140
// 0047cbe6 XOR EDI,EDI
// 0047cbe8 ADD ESP,0xc
// 0047cbeb LEA EBP,[EBX + 0xc]
// 0047cbee MOV dword ptr [EBX + 0x8],EAX
// 0047cbf1 MOV dword ptr [EBP],0x4b7100
// 0047cbf8 MOV dword ptr [EBX + 0x10],0x4b70f0
// 0047cbff MOV dword ptr [EBX],0x47c6f0
// 0047cc05 MOV dword ptr [EBX + 0x4],0x47c6a0
// VC9 /GL /LTCG asmcmp 2026-10-22: 50.00%.
// FUNCTION: VC1 0x4b7100 FOLDED
// SzAlloc

// Ghidra VirtualCity.Unwrapped.exe 0x4b7100..004b710d.
// Upstream 7zAlloc.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// void * SzAllocTemp(uint param0)
//
// {
//   void *pvVar1;
//
//   if (param0 == 0) {
//     return (void *)0x0;
//   }
//   pvVar1 = malloc(param0);
//   return pvVar1;
// }
// ABI / bounds, original assembly:
// 004b7100 MOV EAX,dword ptr [ESP + 0x4]
// 004b7104 TEST EAX,EAX
// 004b7106 JNZ 0x004b7109
// 004b7108 RET
// 004b7109 MOV dword ptr [ESP + 0x4],EAX
// 004b710d JMP 0x0042f6bb
// 004b7100 MOV EAX,dword ptr [ESP + 0x4]
// 004b7104 TEST EAX,EAX
// 004b7106 JNZ 0x004b7109
// 004b7108 RET
// 004b7109 MOV dword ptr [ESP + 0x4],EAX
// 004b710d JMP 0x0042f6bb
// Incoming reference KdConstructNamed:
// 0047cbe0 PUSH EAX
// 0047cbe1 CALL 0x00487140
// 0047cbe6 XOR EDI,EDI
// 0047cbe8 ADD ESP,0xc
// 0047cbeb LEA EBP,[EBX + 0xc]
// 0047cbee MOV dword ptr [EBX + 0x8],EAX
// 0047cbf1 MOV dword ptr [EBP],0x4b7100
// 0047cbf8 MOV dword ptr [EBX + 0x10],0x4b70f0
// 0047cbff MOV dword ptr [EBX],0x47c6f0
// 0047cc05 MOV dword ptr [EBX + 0x4],0x47c6a0
// VC9 /GL /LTCG asmcmp 2026-10-22: 50.00%.
// FUNCTION: VC1 0x4b7100 FOLDED
// SzAllocTemp

// Ghidra VirtualCity.Unwrapped.exe 0x4bc6e0..004bc8ea.
// Upstream 7zExtract.c.obj; candidate from relocated byte pattern.
// Decompiled control flow (original, inferred source identity):
// int SzExtract(_ISzInStream *inStream,_CArchiveDatabaseEx *param1,uint param2,uint *param3,
//              uchar **param4,uint *offset,uint *outSizeProcessed,uint *param7,_ISzAlloc *allocTemp,
//              _ISzAlloc *param9)
//
// {
//   _CFolder *folder;
//   uint uVar1;
//   _CArchiveDatabaseEx *p_Var2;
//   void *param0;
//   int in_EAX;
//   int iVar3;
//   uint uVar4;
//   int iVar5;
//   int iVar6;
//   uint unaff_EBX;
//   uchar *unaff_ESI;
//   _CFolder *unaff_EDI;
//   ulonglong uVar7;
//   _ISzAlloc *in_stack_ffffffdc;
//   uchar *local_10;
//   int local_c;
//
//   iVar5 = *(int *)(*(int *)(in_EAX + 0x3c) + (int)param1 * 4);
//   *offset = 0;
//   *outSizeProcessed = 0;
//   if (iVar5 == -1) {
//     (*(code *)param7[1])(*param3);
//     *param3 = 0;
//     *(undefined4 *)param2 = 0xffffffff;
//     *param4 = (uchar *)0x0;
//     return 0;
//   }
//   if ((*param3 == 0) || (*(int *)param2 != iVar5)) {
//     folder = (_CFolder *)(*(int *)(in_EAX + 0x14) + iVar5 * 0x28);
//     uVar7 = SzFolderGetUnPackSize(unaff_EDI);
//     iVar6 = *(int *)(*(int *)(in_EAX + 0x30) + iVar5 * 4);
//     uVar4 = *(uint *)(*(int *)(in_EAX + 0x34) + iVar6 * 8);
//     uVar1 = *(uint *)(in_EAX + 0x28);
//     iVar6 = *(int *)(*(int *)(in_EAX + 0x34) + 4 + iVar6 * 8) + *(int *)(in_EAX + 0x2c) +
//             (uint)CARRY4(uVar4,*(uint *)(in_EAX + 0x28));
//     local_c = (int)(uVar7 >> 0x20);
//     if (local_c != 0) {
//       return -0x7ff8fff2;
//     }
//     *(int *)param2 = iVar5;
//     (*(code *)param7[1])(*param3);
//     *param3 = 0;
//     iVar3 = (*inStream->Seek)(inStream,uVar4 + uVar1,iVar6);
//     if (iVar3 != 0) {
//       return iVar3;
//     }
//     local_10 = (uchar *)uVar7;
//     *param4 = local_10;
//     if (local_10 != (uchar *)0x0) {
//       uVar4 = (*(code *)*param7)(local_10);
//       *param3 = uVar4;
//       if (uVar4 == 0) {
//         return -0x7ff8fff2;
//       }
//     }
//     iVar6 = SzDecode((ulonglong *)
//                      (*(int *)(in_EAX + 4) + *(int *)(*(int *)(in_EAX + 0x30) + iVar5 * 4) * 8),
//                      folder,inStream,CONCAT44(unaff_EDI,iVar6),unaff_ESI,unaff_EBX,in_stack_ffffffdc
//                     );
//     if (iVar6 != 0) {
//       return iVar6;
//     }
//     if ((folder->UnPackCRCDefined != 0) &&
//        (uVar4 = CrcCalc(local_10,*param3), uVar4 != folder->UnPackCRC)) {
//       return 3;
//     }
//   }
//   iVar6 = (int)param1 * 0x20 + *(int *)(in_EAX + 0x1c);
//   *offset = 0;
//   p_Var2 = *(_CArchiveDatabaseEx **)(*(int *)(in_EAX + 0x38) + iVar5 * 4);
//   if (p_Var2 < param1) {
//     iVar5 = (int)p_Var2 << 5;
//     iVar3 = (int)param1 - (int)p_Var2;
//     do {
//       *offset = *offset + *(int *)(iVar5 + 8 + *(int *)(in_EAX + 0x1c));
//       iVar5 = iVar5 + 0x20;
//       iVar3 = iVar3 + -1;
//     } while (iVar3 != 0);
//   }
//   param0 = *(void **)(iVar6 + 8);
//   *outSizeProcessed = (uint)param0;
//   if (*param4 < (uchar *)((int)param0 + *offset)) {
//     return -0x7fffbffb;
//   }
//   if (*(char *)(iVar6 + 0x18) == '\0') {
//     return 0;
//   }
//   uVar4 = CrcCalc(param0,*param3 + *offset);
//   if (uVar4 != *(uint *)(iVar6 + 0x10)) {
//     return 3;
//   }
//   return 0;
// }
// ABI / bounds, original assembly:
// 004bc6e0 PUSH EBP
// 004bc6e1 MOV EBP,ESP
// 004bc6e3 AND ESP,0xfffffff8
// 004bc6e6 SUB ESP,0x1c
// 004bc6e9 MOV ECX,dword ptr [EBP + 0xc]
// 004bc6ec MOV EDX,dword ptr [EBP + 0x1c]
// 004bc6ef PUSH EBX
// 004bc8e0 MOV EAX,dword ptr [ESP + 0x10]
// 004bc8e4 POP EDI
// 004bc8e5 POP ESI
// 004bc8e6 POP EBX
// 004bc8e7 MOV ESP,EBP
// 004bc8e9 POP EBP
// 004bc8ea RET
// Incoming reference FUN_0047ca30:
// 0047cabe PUSH EAX
// 0047cabf SAR ESI,0x5
// 0047cac2 PUSH ESI
// 0047cac3 LEA EAX,[EBP + 0x18]
// 0047cac6 PUSH EBP
// 0047cac7 MOV dword ptr [ECX],0x0
// 0047cacd CALL 0x004bc6e0
// 0047cad2 ADD ESP,0x28
// 0047cad5 TEST EAX,EAX
// 0047cad7 JZ 0x0047caea
// VC9 /GL /LTCG asmcmp 2026-10-22: 43.13%.
// FUNCTION: VC1 0x4bc6e0
// SzExtract
