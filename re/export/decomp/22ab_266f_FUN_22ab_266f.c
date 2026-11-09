// FUN_22ab_266f @ 22ab:266f

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_22ab_266f(int param_1)

{
  int iVar1;
  int iVar2;
  undefined1 local_14 [16];
  int local_4;
  
  DAT_2cff_b998 = 0x14;
  DAT_2cff_b99a = 0xd;
  while( true ) {
    iVar1 = -1;
    FUN_1123_06c5(1);
    FUN_22ab_001f();
    for (iVar2 = 0; iVar2 < DAT_2cff_bb1c; iVar2 = iVar2 + 1) {
      if ((((undefined1 *)&DAT_2cff_96ac)[iVar2 * 0x1f] == '\f') &&
         (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + iVar2 * 0x1f) == param_1)) {
        iVar1 = iVar2;
      }
    }
    if (iVar1 < 1) break;
    FUN_2ba2_0001(local_14);
    FUN_2ba2_0001(DAT_2cff_b7f2,DAT_2cff_b7f4,(undefined2 *)&DAT_2cff_3a32,0x2cff,0x10);
    *(undefined2 *)((int)_DAT_2cff_b7f2 + 8) =
         *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + iVar1 * 0x1f);
    *(undefined2 *)((int)_DAT_2cff_b7f2 + 10) =
         *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + iVar1 * 0x1f);
    FUN_22ab_14a9();
    FUN_1123_0716();
    FUN_1123_06c5(0);
    FUN_22ab_0006();
    FUN_2ba2_0001(DAT_2cff_b7f2,DAT_2cff_b7f4,local_14);
    if (param_1 == 99) {
      FUN_22ab_2600();
    }
    else {
      local_4 = DAT_2cff_3e28;
      DAT_2cff_3d9e = 1;
LAB_22ab_2784:
      do {
        FUN_1415_026f(0);
        FUN_1581_063e();
        if (DAT_2cff_3c86 == 0) {
          if (DAT_2cff_3c7a == 0) goto LAB_22ab_2784;
        }
      } while (DAT_2cff_3e28 - local_4 < 0x12);
    }
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + iVar1 * 0x1f) == 0) break;
    param_1 = *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + iVar1 * 0x1f);
  }
  DAT_2cff_b998 = 0xf;
  DAT_2cff_b99a = 0xb;
  FUN_1123_06c5(1);
  FUN_22ab_001f();
  FUN_22ab_2523();
  FUN_22ab_14a9();
  FUN_1123_0716();
  FUN_1123_06c5(0);
  FUN_22ab_0006();
  return;
}


