// FUN_1581_0236 @ 1581:0236

void __cdecl16far FUN_1581_0236(void)

{
  int iVar1;
  undefined2 uVar2;
  int iVar3;
  undefined2 in_DX;
  int local_4;
  
  if (DAT_2cff_42f4 == 0) {
    FUN_1530_0440((undefined2 *)&DAT_2cff_4396,0x2cff);
  }
  else {
    DAT_2cff_4396 = 0;
    FUN_1530_0440((undefined2 *)&DAT_2cff_4396,0x2cff);
    iVar1 = FUN_2980_0002();
    if (iVar1 == 0) {
      DAT_2cff_42f4 = 0;
    }
    else if (DAT_2cff_3f0e == 0) {
      DAT_2cff_0aac = 1;
    }
  }
  if (DAT_2cff_3f0e == 0) {
    DAT_2cff_0be6._0_2_ = FUN_2ae9_000a(0x4080);
    DAT_2cff_0be6._2_2_ = in_DX;
    DAT_2cff_0bea._0_2_ = FUN_2ae9_000a(0x4080);
    iVar1 = DAT_2cff_0bee >> 0xf;
    DAT_2cff_0bea._2_2_ = in_DX;
    FUN_2b82_0004(DAT_2cff_439c,DAT_2cff_0bee,iVar1,0);
    for (iVar3 = 0; iVar3 < 0x80; iVar3 = iVar3 + 1) {
      FUN_2b58_000d(DAT_2cff_439c,&local_4);
      if (local_4 == 0) {
        ((undefined2 *)&DAT_2cff_3fdc)[iVar3 * 2] = 0;
        ((undefined2 *)&DAT_2cff_3fda)[iVar3 * 2] = 0;
      }
      else {
        uVar2 = FUN_2ae9_000a(local_4);
        ((undefined2 *)&DAT_2cff_3fdc)[iVar3 * 2] = iVar1;
        ((undefined2 *)&DAT_2cff_3fda)[iVar3 * 2] = uVar2;
        if (((undefined2 *)&DAT_2cff_3fda)[iVar3 * 2] != 0 ||
            ((undefined2 *)&DAT_2cff_3fdc)[iVar3 * 2] != 0) {
          FUN_2b58_000d(DAT_2cff_439c,((undefined2 *)&DAT_2cff_3fda)[iVar3 * 2],
                        ((undefined2 *)&DAT_2cff_3fdc)[iVar3 * 2],local_4);
        }
      }
    }
  }
  else {
    for (iVar1 = 0; iVar1 < 0x32; iVar1 = iVar1 + 1) {
      if (((undefined2 *)&DAT_2cff_3ea6)[iVar1] != 0) {
        uVar2 = FUN_2ae9_000a(((undefined2 *)&DAT_2cff_3ea6)[iVar1]);
        ((undefined2 *)&DAT_2cff_3f12)[iVar1 * 2] = in_DX;
        ((undefined2 *)&DAT_2cff_3f10)[iVar1 * 2] = uVar2;
        if (((undefined2 *)&DAT_2cff_3f10)[iVar1 * 2] != 0 ||
            ((undefined2 *)&DAT_2cff_3f12)[iVar1 * 2] != 0) {
          FUN_2b82_0004(DAT_2cff_439c,((undefined2 *)&DAT_2cff_422a)[iVar1 * 2],
                        ((undefined2 *)&DAT_2cff_422c)[iVar1 * 2],0);
          iVar3 = FUN_2b58_000d(DAT_2cff_439c,((undefined2 *)&DAT_2cff_3f10)[iVar1 * 2],
                                ((undefined2 *)&DAT_2cff_3f12)[iVar1 * 2],
                                ((undefined2 *)&DAT_2cff_3ea6)[iVar1]);
          if (iVar3 == -1) {
            ((undefined2 *)&DAT_2cff_3f12)[iVar1 * 2] = 0;
            ((undefined2 *)&DAT_2cff_3f10)[iVar1 * 2] = 0;
          }
        }
      }
    }
    FUN_15ed_04c5();
  }
  return;
}


