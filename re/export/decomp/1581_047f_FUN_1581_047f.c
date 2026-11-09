// FUN_1581_047f @ 1581:047f

void __cdecl16far FUN_1581_047f(void)

{
  int iVar1;
  
  if (DAT_2cff_3f0e != 0) {
    FUN_15ed_0498();
  }
  for (iVar1 = 0; iVar1 < 0x32; iVar1 = iVar1 + 1) {
    if (((undefined2 *)&DAT_2cff_3f10)[iVar1 * 2] != 0 ||
        ((undefined2 *)&DAT_2cff_3f12)[iVar1 * 2] != 0) {
      FUN_2c09_0008(((undefined2 *)&DAT_2cff_3f10)[iVar1 * 2],
                    ((undefined2 *)&DAT_2cff_3f12)[iVar1 * 2]);
    }
  }
  for (iVar1 = 0; iVar1 < 0x80; iVar1 = iVar1 + 1) {
    if (((undefined2 *)&DAT_2cff_3fda)[iVar1 * 2] != 0 ||
        ((undefined2 *)&DAT_2cff_3fdc)[iVar1 * 2] != 0) {
      FUN_2c09_0008(((undefined2 *)&DAT_2cff_3fda)[iVar1 * 2],
                    ((undefined2 *)&DAT_2cff_3fdc)[iVar1 * 2]);
    }
  }
  FUN_2b53_0006(DAT_2cff_439c);
  if (DAT_2cff_42f4 != 0) {
    FUN_2980_0100();
  }
  FUN_1530_03d8();
  return;
}


