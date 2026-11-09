// FUN_1415_00b6 @ 1415:00b6

undefined2 __cdecl16far
FUN_1415_00b6(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4,
             undefined2 param_5,undefined2 param_6)

{
  int iVar1;
  int iVar2;
  undefined2 uVar3;
  
  uVar3 = 0;
  iVar2 = 0;
  FUN_2c66_0006(param_1,param_2);
  do {
    FUN_1415_0030(param_3,param_4,param_5,param_6);
    iVar1 = FUN_1333_000f();
    if (iVar1 != 0) {
      iVar2 = FUN_1333_0066();
    }
    if (iVar2 == 0x1b) break;
    iVar1 = FUN_1415_0004();
  } while (iVar1 == 0);
  FUN_2c73_0002(0x19);
  if (iVar2 != 0x1b) {
    uVar3 = 1;
    do {
      iVar1 = FUN_1333_000f();
      if (iVar1 != 0) {
        iVar2 = FUN_1333_0066();
      }
      iVar1 = FUN_1415_0004();
    } while ((iVar1 != 0) && (iVar2 != 0x1b));
  }
  FUN_2c73_0002(0x19);
  FUN_2c66_0006(0x4ce,0x2cff);
  return uVar3;
}


