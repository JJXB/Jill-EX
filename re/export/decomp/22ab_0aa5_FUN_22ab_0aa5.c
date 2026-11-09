// FUN_22ab_0aa5 @ 22ab:0aa5

void __cdecl16far FUN_22ab_0aa5(undefined2 param_1,undefined2 param_2)

{
  undefined2 uVar1;
  undefined2 uVar2;
  undefined2 uVar3;
  int iVar4;
  uint uVar5;
  int local_8;
  int local_6;
  int local_4;
  
  for (iVar4 = 9; iVar4 < 0x40; iVar4 = iVar4 + 1) {
    ((undefined2 *)&DAT_2cff_2fae)[iVar4] = 0;
  }
  DAT_2cff_2fca = 1;
  DAT_2cff_300a = 1;
  FUN_2b9f_0008((undefined1 *)&DAT_2cff_bafc,0x2cff,param_1,param_2);
  FUN_22ab_090d();
  uVar1 = FUN_2b4b_000c(param_1,param_2,0x8000);
  iVar4 = FUN_2b58_000d(uVar1,(undefined2 *)&DAT_2cff_43a6,0x2cff,0x4000);
  if (iVar4 == 0) {
    FUN_22ab_3177(1);
  }
  iVar4 = FUN_2b58_000d(uVar1,(undefined2 *)&DAT_2cff_bb1c,0x2cff,2);
  if (iVar4 == 0) {
    FUN_22ab_3177(2);
  }
  iVar4 = FUN_2b58_000d(uVar1,(undefined1 *)&DAT_2cff_96ac,0x2cff,DAT_2cff_bb1c * 0x1f);
  if (iVar4 == 0) {
    FUN_22ab_3177(3);
  }
  iVar4 = FUN_2b58_000d(uVar1,(undefined2 *)&DAT_2cff_83a6,0x2cff,0x46);
  if (iVar4 == 0) {
    FUN_22ab_3177(4);
  }
  for (uVar5 = 0; (int)uVar5 < DAT_2cff_bb1c; uVar5 = uVar5 + 1) {
    uVar3 = (undefined2)((ulong)uVar5 * 0x1f >> 0x10);
    iVar4 = (int)((ulong)uVar5 * 0x1f);
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar4) != 0 ||
        *(int *)((int)(undefined2 *)&DAT_2cff_96c5 + iVar4) != 0) {
      FUN_2b58_000d(uVar1,&local_8);
      uVar2 = FUN_2ae9_000a(local_8 + 1);
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c5 + uVar5 * 0x1f) = uVar3;
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c3 + uVar5 * 0x1f) = uVar2;
      FUN_2b58_000d(uVar1,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c3 + uVar5 * 0x1f),
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c5 + uVar5 * 0x1f),local_8 + 1);
    }
  }
  FUN_2b56_0007(uVar1);
  for (local_6 = 0; local_6 < 0x80; local_6 = local_6 + 1) {
    for (local_4 = 0; local_4 < 0x40; local_4 = local_4 + 1) {
      ((undefined2 *)&DAT_2cff_2fae)
      [*(int *)(((undefined2 *)&DAT_2cff_43a6)[local_6 * 0x40 + local_4] * 8 + -0x7c14) >> 8 & 0x3f]
           = 1;
    }
  }
  for (iVar4 = 0; iVar4 < DAT_2cff_bb1c; iVar4 = iVar4 + 1) {
    ((undefined2 *)&DAT_2cff_2fae)
    [((undefined2 *)&DAT_2cff_bb3e)[(char)((undefined1 *)&DAT_2cff_96ac)[iVar4 * 0x1f]]] = 1;
  }
  FUN_1069_094d();
  return;
}


