// FUN_22ab_090d @ 22ab:090d

void __cdecl16far FUN_22ab_090d(void)

{
  int iVar1;
  
  for (iVar1 = 0; iVar1 < DAT_2cff_bb1c; iVar1 = iVar1 + 1) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar1 * 0x1f) != 0 ||
        *(int *)((int)(undefined2 *)&DAT_2cff_96c5 + iVar1 * 0x1f) != 0) {
      FUN_2c09_0008(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar1 * 0x1f),
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c5 + iVar1 * 0x1f));
    }
  }
  FUN_26f4_0008();
  return;
}


