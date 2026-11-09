// FUN_26f4_2506 @ 26f4:2506

void __cdecl16far FUN_26f4_2506(void)

{
  int iVar1;
  int iVar2;
  
  iVar2 = 0;
  for (iVar1 = 0; iVar1 < DAT_2cff_bb1c; iVar1 = iVar1 + 1) {
    if (iVar1 != iVar2) {
      FUN_2ba2_0001((undefined1 *)&DAT_2cff_96ac + iVar2 * 0x1f,0x2cff,
                    (undefined1 *)&DAT_2cff_96ac + iVar1 * 0x1f,0x2cff,0x1f);
    }
    if (((undefined1 *)&DAT_2cff_96ac)[iVar1 * 0x1f] == '\x03') {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar1 * 0x1f) != 0 ||
          *(int *)((int)(undefined2 *)&DAT_2cff_96c5 + iVar1 * 0x1f) != 0) {
        FUN_2c09_0008(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar1 * 0x1f),
                      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c5 + iVar1 * 0x1f));
      }
    }
    else {
      iVar2 = iVar2 + 1;
    }
  }
  DAT_2cff_bb1c = iVar2;
  return;
}


