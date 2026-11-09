// FUN_1123_063d @ 1123:063d

void __cdecl16far FUN_1123_063d(void)

{
  int iVar1;
  
  for (iVar1 = 0; iVar1 < 0x10; iVar1 = iVar1 + 1) {
    ((undefined2 *)&DAT_2cff_3232)[iVar1] = iVar1;
    ((undefined2 *)&DAT_2cff_3632)[iVar1] = 0;
    ((undefined2 *)&DAT_2cff_3832)[iVar1] = iVar1;
  }
  DAT_2cff_3232 = 0x10;
  DAT_2cff_3632 = 0x10;
  return;
}


