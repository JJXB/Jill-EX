// FUN_26f4_03c4 @ 26f4:03c4

void __cdecl16far FUN_26f4_03c4(void)

{
  char cVar1;
  int iVar2;
  
  iVar2 = FUN_26f4_037d(DAT_2cff_83a6);
  if (((iVar2 < 1) ||
      (*(int *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar2 * 0x1f) == 0 &&
       *(int *)((int)(undefined2 *)&DAT_2cff_96c5 + iVar2 * 0x1f) == 0)) ||
     ((*(char *)*(undefined4 *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar2 * 0x1f) != '*' &&
      ((*(char *)*(undefined4 *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar2 * 0x1f) != '#' &&
       (*(char *)*(undefined4 *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar2 * 0x1f) != '&')))))) {
    iVar2 = FUN_26f4_037d(0);
    cVar1 = *(char *)*(undefined4 *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar2 * 0x1f);
    if (((cVar1 == '*') || (cVar1 == '#')) || (cVar1 == '&')) {
      FUN_2b9f_0008((undefined1 *)&DAT_2cff_bb1e,0x2cff,
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar2 * 0x1f),
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c5 + iVar2 * 0x1f));
    }
  }
  else {
    FUN_2b9f_0008((undefined1 *)&DAT_2cff_bb1e,0x2cff,
                  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c3 + iVar2 * 0x1f),
                  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c5 + iVar2 * 0x1f));
  }
  return;
}


