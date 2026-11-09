// FUN_1123_0674 @ 1123:0674

void __cdecl16far FUN_1123_0674(void)

{
  int iVar1;
  
  for (iVar1 = 0; iVar1 < 0x100; iVar1 = iVar1 + 1) {
    ((undefined2 *)&DAT_2cff_3232)[iVar1] = iVar1;
    ((undefined2 *)&DAT_2cff_3632)[iVar1] = 0;
    ((undefined2 *)&DAT_2cff_3832)[iVar1] = iVar1;
  }
  DAT_2cff_3232 = 0xff;
  DAT_2cff_3632 = 0xff;
  return;
}


