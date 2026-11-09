// FUN_1415_06a4 @ 1415:06a4

void __cdecl16far FUN_1415_06a4(undefined2 param_1,undefined2 param_2)

{
  int in_DX;
  
  FUN_1415_05b2();
  DAT_2cff_04c0 = FUN_2ae9_000a(8000);
  DAT_2cff_04c2 = in_DX;
  if (DAT_2cff_04c0 != 0 || in_DX != 0) {
    DAT_2cff_3d9c = 0;
    DAT_2cff_3dc2 = 1;
    FUN_2b9f_0008(0x3da2,0x2cff,param_1,param_2);
    DAT_2cff_be3a = 0;
  }
  return;
}


