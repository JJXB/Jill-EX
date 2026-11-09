// FUN_26f4_037d @ 26f4:037d

int __cdecl16far FUN_26f4_037d(int param_1)

{
  int iVar1;
  
  iVar1 = 0;
  while( true ) {
    if (DAT_2cff_bb1c <= iVar1) {
      return 0;
    }
    if ((((undefined1 *)&DAT_2cff_96ac)[iVar1 * 0x1f] == '\f') &&
       (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + iVar1 * 0x1f) == param_1)) break;
    iVar1 = iVar1 + 1;
  }
  return iVar1;
}


