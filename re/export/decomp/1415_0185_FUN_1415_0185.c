// FUN_1415_0185 @ 1415:0185

undefined2 __cdecl16far FUN_1415_0185(void)

{
  int iVar1;
  undefined2 uVar2;
  
  while( true ) {
    DAT_2cff_3d9a = 0;
    FUN_2c66_0006(0x4d1,0x2cff);
    iVar1 = FUN_1415_00b6(0x503,0x2cff,(undefined2 *)&DAT_2cff_3c80,0x2cff,
                          (undefined2 *)&DAT_2cff_3c82,0x2cff);
    if (iVar1 == 0) {
      return 0;
    }
    iVar1 = FUN_1415_00b6(0x528,0x2cff,(undefined2 *)&DAT_2cff_3c8c,0x2cff,
                          (undefined2 *)&DAT_2cff_3d94,0x2cff);
    if (iVar1 == 0) {
      return 0;
    }
    iVar1 = FUN_1415_00b6(0x560,0x2cff,(undefined2 *)&DAT_2cff_3d8e,0x2cff,
                          (undefined2 *)&DAT_2cff_3c84,0x2cff);
    if (iVar1 == 0) {
      return 0;
    }
    DAT_2cff_3c8c = DAT_2cff_3c8c - DAT_2cff_3c80;
    DAT_2cff_3d8e = DAT_2cff_3d8e - DAT_2cff_3c80;
    DAT_2cff_3d94 = DAT_2cff_3d94 - DAT_2cff_3c82;
    DAT_2cff_3c84 = DAT_2cff_3c84 - DAT_2cff_3c82;
    if ((((DAT_2cff_3c8c < -1) && (1 < DAT_2cff_3d8e)) && (DAT_2cff_3d94 < -1)) &&
       (1 < DAT_2cff_3c84)) break;
    FUN_2c66_0006(0x599,0x2cff);
    do {
      iVar1 = FUN_1333_000f();
    } while (iVar1 == 0);
    FUN_2c66_0006(0x5c2,0x2cff);
    uVar2 = FUN_1333_0066();
    iVar1 = FUN_2ce7_000d(uVar2);
    if (iVar1 != 0x59) {
      return 0;
    }
  }
  return 1;
}


