// FUN_26f4_1a38 @ 26f4:1a38

undefined2 __cdecl16far FUN_26f4_1a38(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = 0;
  while( true ) {
    if (DAT_2cff_83aa <= iVar1) {
      return 0;
    }
    if (((undefined2 *)&DAT_2cff_83ac)[iVar1] == param_1) break;
    iVar1 = iVar1 + 1;
  }
  while (iVar2 = iVar1 + 1, iVar2 < DAT_2cff_83aa) {
    ((undefined2 *)&DAT_2cff_83ac)[iVar1] = ((undefined2 *)&DAT_2cff_83ac)[iVar2];
    iVar1 = iVar2;
  }
  DAT_2cff_83aa = DAT_2cff_83aa + -1;
  DAT_2cff_be38 = DAT_2cff_be38 | 0xc000;
  return 1;
}


