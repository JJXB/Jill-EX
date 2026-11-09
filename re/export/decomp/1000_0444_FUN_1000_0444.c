// FUN_1000_0444 @ 1000:0444

void __cdecl16far FUN_1000_0444(byte param_1)

{
  undefined2 uVar1;
  int iVar2;
  
  if ((3 < param_1) && (param_1 != 7)) {
    param_1 = 3;
  }
  DAT_2cff_2a30 = param_1;
  uVar1 = FUN_1000_0414();
  if ((byte)uVar1 != DAT_2cff_2a30) {
    FUN_1000_0414();
    uVar1 = FUN_1000_0414();
    DAT_2cff_2a30 = (byte)uVar1;
  }
  DAT_2cff_2a32 = (char)((uint)uVar1 >> 8);
  if ((DAT_2cff_2a30 < 4) || (DAT_2cff_2a30 == 7)) {
    DAT_2cff_2a33 = 0;
  }
  else {
    DAT_2cff_2a33 = 1;
  }
  DAT_2cff_2a31 = 0x19;
  if (DAT_2cff_2a30 != 7) {
    iVar2 = FUN_1000_03d0(0x2a3b,0x2cff,0xffea,0xf000);
    if (iVar2 == 0) {
      iVar2 = FUN_1000_03ff();
      if (iVar2 == 0) {
        DAT_2cff_2a34 = 1;
        goto LAB_1000_04bf;
      }
    }
  }
  DAT_2cff_2a34 = 0;
LAB_1000_04bf:
  if (DAT_2cff_2a30 == 7) {
    DAT_2cff_2a37 = 0xb000;
  }
  else {
    DAT_2cff_2a37 = 0xb800;
  }
  DAT_2cff_2a35 = 0;
  DAT_2cff_2a2b = 0;
  DAT_2cff_2a2a = 0;
  DAT_2cff_2a2c = DAT_2cff_2a32 + -1;
  DAT_2cff_2a2d = 0x18;
  return;
}


