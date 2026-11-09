// FUN_1353_07f1 @ 1353:07f1

void __cdecl16far
FUN_1353_07f1(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
             undefined2 param_5)

{
  int iVar1;
  byte local_4 [2];
  
  local_4[1] = 0;
  while( true ) {
    iVar1 = FUN_1333_000f();
    if (iVar1 != 0) break;
    FUN_2c73_0002(100);
    DAT_2cff_3c70 = (DAT_2cff_3c70 & 7) + 1;
    local_4[0] = DAT_2cff_3c70;
    FUN_1353_074e(param_1,param_2,param_3,param_4,param_5,local_4);
  }
  FUN_1353_074e(param_1,param_2,param_3,param_4,param_5,0x4be,0x2cff);
  FUN_1333_0066();
  return;
}


