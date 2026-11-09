// FUN_14cc_0004 @ 14cc:0004

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 __cdecl16far FUN_14cc_0004(void)

{
  undefined2 uVar1;
  int iVar2;
  undefined2 in_DX;
  undefined1 local_12 [16];
  
  iVar2 = DAT_2cff_3dce;
  if (DAT_2cff_3dce == 0) {
    DAT_2cff_3a4d = DAT_2cff_3dde;
    DAT_2cff_3d9a = FUN_1415_0149();
    if (DAT_2cff_3d9a == 0) {
      DAT_2cff_3dd0 = 0;
    }
    else if (DAT_2cff_3dd0 != 0) {
      DAT_2cff_3c8c = DAT_2cff_3dd2;
      DAT_2cff_3c80 = DAT_2cff_3dd4;
      DAT_2cff_3d8e = DAT_2cff_3dd6;
      DAT_2cff_3d94 = DAT_2cff_3dd8;
      DAT_2cff_3c82 = DAT_2cff_3dda;
      DAT_2cff_3c84 = DAT_2cff_3ddc;
      FUN_1415_026f(0);
      if ((DAT_2cff_3c76 == 0) && (DAT_2cff_3c78 == 0)) {
        iVar2 = 0;
      }
      else {
        iVar2 = 1;
      }
    }
    if (DAT_2cff_42f4 == 0) {
      DAT_2cff_3de0 = 0;
    }
    if (DAT_2cff_3f0e == 0) {
      DAT_2cff_3de2 = 0;
    }
  }
  if (iVar2 == 0) {
    FUN_2c48_0006();
    FUN_2c66_0006(0x5f6,0x2cff);
    FUN_2c66_0006(0x5f9,0x2cff);
    if (DAT_2cff_3de2 == 0) {
      FUN_2c66_0006(0x63d,0x2cff);
    }
    else {
      FUN_2c66_0006(0x610,0x2cff);
    }
    if (DAT_2cff_3de0 == 0) {
      FUN_2c66_0006(0x689,0x2cff);
    }
    else {
      FUN_2c66_0006(0x65e,0x2cff);
    }
    if (DAT_2cff_3dd0 == 0) {
      FUN_2c66_0006(0x6b7,0x2cff);
    }
    else {
      FUN_2c66_0006(0x6a6,0x2cff);
    }
    if (DAT_2cff_3a4d == 0) {
      FUN_2c66_0006(0x6c9,0x2cff);
      FUN_2c66_0006(0x6f1,0x2cff);
    }
    else if (DAT_2cff_3a4d == 2) {
      FUN_2c66_0006(0x713,0x2cff);
    }
    else {
      FUN_2c66_0006(0x72f,0x2cff);
    }
    FUN_2c66_0006(0x74c,0x2cff);
    FUN_2c66_0006(0x74f,0x2cff);
    FUN_2c66_0006(0x772,0x2cff);
    do {
      FUN_1415_059e();
      DAT_2cff_3c86 = FUN_2ce7_000d(DAT_2cff_3c86);
      if ((DAT_2cff_3c86 == 0xd) || (DAT_2cff_3c86 == 0x43)) break;
    } while (DAT_2cff_3c86 != 0x1b);
    if (DAT_2cff_3c86 == 0x43) {
      iVar2 = 1;
    }
    if (DAT_2cff_3c86 == 0x1b) {
      return 0;
    }
  }
  if (iVar2 != 0) {
    FUN_2c48_0006();
    if ((DAT_2cff_3f0e == 0) && (DAT_2cff_42f4 == 0)) {
      FUN_2c66_0006(0x794,0x2cff);
      FUN_2c66_0006(0x797,0x2cff);
      FUN_2c66_0006(0x7ca,0x2cff);
      FUN_2c66_0006(0x7d9,0x2cff);
      FUN_1415_059e();
    }
    if (DAT_2cff_3f0e != 0) {
      FUN_2c66_0006(0x7f7,0x2cff);
      FUN_2c66_0006(0x824,0x2cff);
      FUN_2c66_0006(0x856,0x2cff);
      FUN_2c66_0006(0x883,0x2cff);
      FUN_2c66_0006(0x8b7,0x2cff);
      FUN_2c66_0006(0x8e9,0x2cff);
      FUN_2c66_0006(0x91a,0x2cff);
      do {
        FUN_1415_059e();
        DAT_2cff_3c86 = FUN_2ce7_000d(DAT_2cff_3c86);
        if (DAT_2cff_3c86 == 0x7e) {
          uVar1 = FUN_2bb2_0553(local_12);
          FUN_2b85_00d2(uVar1,in_DX);
          FUN_2c66_0006(local_12);
        }
        if (DAT_2cff_3c86 == 0x1b) {
          return 0;
        }
      } while ((DAT_2cff_3c86 != 0x59) && (DAT_2cff_3c86 != 0x4e));
      DAT_2cff_3de2 = (uint)(DAT_2cff_3c86 == 0x59);
    }
    if (DAT_2cff_42f4 != 0) {
      FUN_2c48_0006();
      FUN_2c66_0006(0x937,0x2cff);
      FUN_2c66_0006(0x93e,0x2cff);
      FUN_2c66_0006(0x96f,0x2cff);
      FUN_2c66_0006(0x98b,0x2cff);
      do {
        FUN_1415_059e();
        DAT_2cff_3c86 = FUN_2ce7_000d(DAT_2cff_3c86);
        if (DAT_2cff_3c86 == 0x1b) {
          return 0;
        }
      } while ((DAT_2cff_3c86 != 0x59) && (DAT_2cff_3c86 != 0x4e));
      DAT_2cff_3de0 = (uint)(DAT_2cff_3c86 == 0x59);
    }
    FUN_2c48_0006();
    FUN_2c66_0006(0x9b2,0x2cff);
    iVar2 = FUN_1415_0528();
    if (iVar2 == 0) {
      return 0;
    }
    DAT_2cff_3dd0 = DAT_2cff_3d9a;
    FUN_2c48_0006();
    FUN_2c66_0006(0x9b5,0x2cff);
    FUN_2c66_0006(0x9b8,0x2cff);
    FUN_2c66_0006(0x9df,0x2cff);
    FUN_2c66_0006(0x9fb,0x2cff);
    FUN_2c66_0006(0xa18,0x2cff);
    FUN_2c66_0006(0xa36,0x2cff);
    FUN_2c66_0006(0xa39,0x2cff);
    FUN_2c66_0006(0xa67,0x2cff);
    iVar2 = FUN_1123_0a21();
    if (iVar2 == 0) {
      return 0;
    }
  }
  DAT_2cff_42f4 = DAT_2cff_3de0;
  DAT_2cff_3f0e = DAT_2cff_3de2;
  _DAT_2cff_3dde = (uint)DAT_2cff_3a4d;
  DAT_2cff_3ddc = DAT_2cff_3c84;
  DAT_2cff_3dda = DAT_2cff_3c82;
  DAT_2cff_3dd8 = DAT_2cff_3d94;
  DAT_2cff_3dd6 = DAT_2cff_3d8e;
  DAT_2cff_3dd4 = DAT_2cff_3c80;
  DAT_2cff_3dd2 = DAT_2cff_3c8c;
  DAT_2cff_3dce = 0;
  DAT_2cff_3d9a = DAT_2cff_3dd0;
  return 1;
}


