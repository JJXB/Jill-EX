// FUN_1530_0003 @ 1530:0003

void __cdecl16far FUN_1530_0003(int param_1,undefined4 param_2,int param_3,undefined2 param_4)

{
  int iVar1;
  undefined2 uVar2;
  int iVar3;
  
  if ((DAT_2cff_0aac == 0) &&
     ((DAT_2cff_3e2c == 0 ||
      (((DAT_2cff_3e3e <= param_1 && (DAT_2cff_3e3e != -1)) || (param_1 == -1)))))) {
    if ((-1 < param_1) || (DAT_2cff_3e2c == 0)) {
      DAT_2cff_3e3e = param_1;
      DAT_2cff_3e30 = 0;
      DAT_2cff_3e2e = 0;
      DAT_2cff_3e38 = 0;
    }
    iVar3 = 0;
    DAT_2cff_3e2c = 1;
    do {
      iVar1 = *(int *)((int)param_2 + iVar3 * 2);
      iVar3 = iVar3 + 1;
      if ((iVar1 < 0) && (iVar1 == -1)) {
        *(undefined2 *)((int)DAT_2cff_0be6 + DAT_2cff_3e2e * 2) = 0xffff;
      }
      else {
        FUN_1000_0390();
        uVar2 = FUN_1000_0595();
        *(undefined2 *)((int)DAT_2cff_0be6 + DAT_2cff_3e2e * 2) = uVar2;
      }
      *(undefined2 *)((int)DAT_2cff_0bea + DAT_2cff_3e2e * 2) = param_4;
      DAT_2cff_3e2e = DAT_2cff_3e2e + 1;
    } while ((iVar3 < param_3) && (DAT_2cff_3e2e < 0x2000));
  }
  return;
}


