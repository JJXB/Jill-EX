// FUN_2b1a_000a @ 2b1a:000a

undefined2 FUN_2b1a_000a(undefined4 param_1)

{
  uint uVar1;
  undefined2 uVar2;
  int iVar3;
  
  uVar1 = (param_1._2_2_ - DAT_2cff_007b) + 0x40U >> 6;
  if (uVar1 == DAT_2cff_2850) {
    DAT_2cff_008b._2_2_ = param_1._2_2_;
    uVar2 = 1;
    DAT_2cff_008b._0_2_ = (int)param_1;
  }
  else {
    uVar1 = uVar1 * 0x40;
    if (DAT_2cff_0091 < uVar1 + DAT_2cff_007b) {
      uVar1 = DAT_2cff_0091 - DAT_2cff_007b;
    }
    iVar3 = FUN_2b2f_000c(DAT_2cff_007b,uVar1);
    if (iVar3 == -1) {
      DAT_2cff_2850 = uVar1 >> 6;
      DAT_2cff_008b._2_2_ = param_1._2_2_;
      uVar2 = 1;
      DAT_2cff_008b._0_2_ = (int)param_1;
    }
    else {
      DAT_2cff_0091 = DAT_2cff_007b + iVar3;
      DAT_2cff_008f = 0;
      uVar2 = 0;
    }
  }
  return uVar2;
}


