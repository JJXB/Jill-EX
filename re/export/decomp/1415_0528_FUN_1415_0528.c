// FUN_1415_0528 @ 1415:0528

bool __cdecl16far FUN_1415_0528(void)

{
  int iVar1;
  undefined2 uVar2;
  int iVar3;
  
  iVar3 = 0x20;
  iVar1 = FUN_1415_0149();
  if (iVar1 != 0) {
    FUN_2c66_0006(0x5c5,0x2cff);
    do {
      do {
        iVar1 = FUN_1333_000f();
      } while (iVar1 == 0);
      uVar2 = FUN_1333_0066();
      iVar3 = FUN_2ce7_000d(uVar2);
    } while (((iVar3 != 0x4b) && (iVar3 != 0x4a)) && (iVar3 != 0x1b));
    FUN_2c66_0006(0x5f2,0x2cff);
    DAT_2cff_3d9a = 0;
    if (iVar3 == 0x4a) {
      DAT_2cff_3d9a = FUN_1415_0185();
    }
  }
  return iVar3 != 0x1b;
}


