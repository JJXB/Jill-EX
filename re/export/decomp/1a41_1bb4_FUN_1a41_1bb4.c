// FUN_1a41_1bb4 @ 1a41:1bb4

/* WARNING: Unable to track spacebase fully for stack */

uint __cdecl16far FUN_1a41_1bb4(undefined1 *param_1,uint param_2)

{
  undefined1 *puVar1;
  code *pcVar2;
  undefined4 uVar3;
  char cVar4;
  byte extraout_AH;
  undefined2 uVar5;
  uint uVar6;
  byte bVar7;
  int iVar8;
  char cVar9;
  undefined2 extraout_DX;
  undefined2 extraout_DX_00;
  int iVar10;
  undefined1 *puVar11;
  undefined1 *puVar12;
  undefined1 *puVar13;
  undefined1 *puVar14;
  undefined1 *puVar15;
  int unaff_DI;
  undefined4 *puVar16;
  undefined2 unaff_ES;
  undefined2 unaff_SS;
  undefined2 uVar17;
  undefined1 local_2e [22];
  undefined1 local_18 [6];
  undefined1 auStack_12 [16];
  
  puVar16 = (undefined4 *)(DAT_2cff_bb96 << 8);
  FUN_1000_05d8(0xf88,0x2cff,local_2e,unaff_SS);
  bVar7 = 0x16;
  FUN_1000_05d8(0xf9e,0x2cff,local_18,unaff_SS);
  if (5 < param_2) {
    return param_2;
  }
  iVar10 = param_2 * 2;
  switch(param_2) {
  case 0:
    *(undefined1 *)puVar16 = *param_1;
    uVar17 = extraout_DX_00;
    FUN_2ca4_000c(0x3d4,0xe317);
    FUN_11f4_0e77((undefined2 *)&DAT_2cff_3a32,0x2cff,(uint)extraout_AH << 8);
    FUN_1123_0791();
    DAT_2cff_322e = FUN_2ae9_000a(1);
    DAT_2cff_3230 = uVar17;
    return DAT_2cff_322e;
  case 1:
    uVar6 = FUN_1353_074e(DAT_2cff_b7f2,DAT_2cff_b7f4,
                          *(int *)((int)(undefined2 *)&DAT_2cff_96ad + (int)param_1 * 0x1f) + 4,
                          *(int *)((int)(undefined2 *)&DAT_2cff_96af + (int)param_1 * 0x1f) + 4,1,
                          auStack_12);
    return uVar6;
  case 2:
    pcVar2 = (code *)swi(0x21);
    uVar6 = (*pcVar2)();
    return uVar6;
  case 3:
    unaff_DI = unaff_DI << (bVar7 & 0x1f);
    puVar15 = (undefined1 *)&DAT_2cff_2a5c;
    for (iVar8 = -0x6b08; iVar8 != 0; iVar8 = iVar8 + -1) {
      puVar1 = puVar15;
      puVar15 = puVar15 + 1;
      *puVar1 = 0;
    }
    *(undefined2 *)(unaff_DI + -2) = 0x1000;
    pcVar2 = DAT_2cff_2a46;
    puVar11 = (undefined1 *)(unaff_DI + -4);
    *(undefined2 *)(unaff_DI + -4) = 0xd3;
    (*pcVar2)();
                    /* WARNING: Call to offcut address within same function */
    *(undefined2 *)(puVar11 + -2) = 0x1000;
    puVar12 = puVar11 + -4;
    *(undefined2 *)(puVar11 + -4) = 0xd8;
    func_0x000101d2();
    *(undefined2 *)(puVar12 + -2) = 0x1000;
    *(undefined2 *)(puVar12 + -4) = 0xdd;
    FUN_1000_02d0();
    pcVar2 = (code *)swi(0x1a);
    (*pcVar2)();
    DAT_2cff_0083 = extraout_DX;
    DAT_2cff_0085 = iVar8;
    *(undefined2 *)(puVar12 + -2) = 0x1000;
    pcVar2 = DAT_2cff_2a4a;
    puVar13 = puVar12 + -4;
    *(undefined2 *)(puVar12 + -4) = 0xee;
    (*pcVar2)();
    *(undefined2 *)(puVar13 + -2) = DAT_2cff_0073;
    *(undefined2 *)(puVar13 + -4) = DAT_2cff_0071;
    *(undefined2 *)(puVar13 + -6) = DAT_2cff_006f;
    *(undefined2 *)(puVar13 + -8) = DAT_2cff_006d;
    *(undefined2 *)(puVar13 + -10) = DAT_2cff_006b;
    *(undefined2 *)(puVar13 + -0xc) = 0x1000;
    *(undefined2 *)(puVar13 + -0xe) = 0x107;
    uVar17 = FUN_22ab_2ed8();
    *(undefined2 *)(puVar13 + -0xc) = uVar17;
    *(undefined2 *)(puVar13 + -0xe) = 0x22ab;
    *(undefined2 *)(puVar13 + -0x10) = 0x10d;
    FUN_2ae3_0008();
    *(undefined2 *)(puVar13 + -0xe) = 0x1000;
    *(undefined2 *)(puVar13 + -0x10) = 0x117;
    FUN_1000_0172();
    *(undefined2 *)(puVar13 + -0xe) = 0x1000;
    pcVar2 = pcRam00002a48;
    puVar14 = puVar13 + -0x10;
    *(undefined2 *)(puVar13 + -0x10) = 0x11c;
    (*pcVar2)();
    pcVar2 = (code *)swi(0x21);
    (*pcVar2)();
    cVar9 = '\0';
    *(undefined2 *)(puVar14 + -2) = 0x1be;
    FUN_1000_01a7();
    *(undefined2 *)(puVar14 + -2) = 3;
    *(undefined2 *)(puVar14 + -4) = 0x1000;
    *(undefined2 *)(puVar14 + -6) = 0x1c7;
    uVar6 = FUN_1000_010d();
    cVar4 = (char)uVar6;
    param_1[iVar10] = param_1[iVar10] + cVar4;
    param_1[iVar10] = param_1[iVar10] + cVar4;
    param_1[iVar10] = param_1[iVar10] + cVar4;
    param_1[iVar10] = param_1[iVar10] + cVar4;
    param_1[iVar10] = param_1[iVar10] + cVar4;
    cRam0000068f = cRam0000068f + cVar9;
    return uVar6;
  case 4:
    uVar3 = *puVar16;
    uVar17 = (undefined2)((ulong)uVar3 >> 0x10);
    FUN_2c66_0006((int)uVar3);
    break;
  case 5:
    uVar6 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                          *(int *)((int)(undefined2 *)&DAT_2cff_96bf + (int)param_1 * 0x1f) / 2 +
                          0xe06,*(undefined2 *)
                                 ((int)(undefined2 *)&DAT_2cff_96ad + (int)param_1 * 0x1f),
                          *(undefined2 *)(iVar10 + 3));
    return uVar6;
  }
  do {
    do {
      iVar10 = FUN_1333_000f();
    } while (iVar10 == 0);
    uVar5 = FUN_1333_0066();
    iVar10 = FUN_2ce7_000d(uVar5);
  } while (((iVar10 != 0x4b) && (iVar10 != 0x4a)) && (iVar10 != 0x1b));
  FUN_2c66_0006(0x5f2,uVar17);
  *(undefined2 *)&DAT_2cff_3d9a = 0;
  if (iVar10 == 0x4a) {
    uVar5 = FUN_1415_0185();
    *(undefined2 *)&DAT_2cff_3d9a = uVar5;
  }
  return (uint)(iVar10 != 0x1b);
}


