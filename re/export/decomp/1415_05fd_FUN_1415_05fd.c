// FUN_1415_05fd @ 1415:05fd

void __cdecl16far FUN_1415_05fd(undefined2 param_1,undefined2 param_2)

{
  int iVar1;
  int iVar2;
  int in_DX;
  
  FUN_1415_05b2();
  DAT_2cff_3dcc = 0;
  iVar1 = FUN_2b4b_000c(param_1,param_2,0x8000);
  if (-1 < iVar1) {
    DAT_2cff_3d98 = FUN_2c44_0001(iVar1);
    DAT_2cff_04c0 = FUN_2ae9_000a(DAT_2cff_3d98);
    if (DAT_2cff_04c0 == 0 && in_DX == 0) {
      DAT_2cff_04c2 = 0;
      DAT_2cff_04c0 = 0;
    }
    else {
      DAT_2cff_04c2 = in_DX;
      iVar2 = FUN_2b65_0002(iVar1,DAT_2cff_04c0,in_DX,DAT_2cff_3d98);
      if (iVar2 < 0) {
        FUN_2c09_0008(DAT_2cff_04c0,DAT_2cff_04c2);
        DAT_2cff_04c2 = 0;
        DAT_2cff_04c0 = 0;
      }
      else {
        DAT_2cff_3dc8 = 1;
        DAT_2cff_be3a = 0;
      }
    }
    FUN_2b56_0007(iVar1);
  }
  return;
}


