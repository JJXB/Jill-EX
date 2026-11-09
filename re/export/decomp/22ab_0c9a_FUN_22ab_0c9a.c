// FUN_22ab_0c9a @ 22ab:0c9a

void __cdecl16far FUN_22ab_0c9a(undefined2 param_1,undefined2 param_2)

{
  undefined2 uVar1;
  int iVar2;
  int local_4;
  
  uVar1 = FUN_2c3b_003d(param_1,param_2,0);
  iVar2 = FUN_2b67_0002(uVar1,(undefined2 *)&DAT_2cff_43a6,0x2cff,0x4000);
  if (iVar2 == 0) {
    FUN_22ab_3177(5);
  }
  FUN_2b67_0002(uVar1,(undefined2 *)&DAT_2cff_bb1c,0x2cff,2);
  FUN_2b67_0002(uVar1,(undefined1 *)&DAT_2cff_96ac,0x2cff,DAT_2cff_bb1c * 0x1f);
  FUN_2b67_0002(uVar1,(undefined2 *)&DAT_2cff_83a6,0x2cff,0x46);
  for (iVar2 = 0; iVar2 < DAT_2cff_bb1c; iVar2 = iVar2 + 1) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar2 * 0x1f) != 0 ||
        *(int *)((int)(undefined2 *)&DAT_2cff_96c5 + iVar2 * 0x1f) != 0) {
      local_4 = FUN_2b9a_000b(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar2 * 0x1f),
                              *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c5 + iVar2 * 0x1f));
      FUN_2b67_0002(uVar1,&local_4);
      FUN_2b67_0002(uVar1,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar2 * 0x1f),
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c5 + iVar2 * 0x1f),local_4 + 1);
    }
  }
  FUN_2b56_0007(uVar1);
  return;
}


