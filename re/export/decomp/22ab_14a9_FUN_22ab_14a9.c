// FUN_22ab_14a9 @ 22ab:14a9

void __cdecl16far FUN_22ab_14a9(void)

{
  int iVar1;
  int iVar2;
  
  for (iVar1 = 0; iVar1 < 0x80; iVar1 = iVar1 + 1) {
    for (iVar2 = 0; iVar2 < 0x40; iVar2 = iVar2 + 1) {
      ((undefined2 *)&DAT_2cff_43a6)[iVar1 * 0x40 + iVar2] =
           ((undefined2 *)&DAT_2cff_43a6)[iVar1 * 0x40 + iVar2] | 0xc000;
    }
  }
  FUN_26f4_12b9(0);
  DAT_2cff_be38 = 0;
  FUN_26f4_0d52(0);
  return;
}


