// FUN_1581_0009 @ 1581:0009

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_1581_0009(char *param_1,undefined2 param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  DAT_2cff_4396 = param_2;
  DAT_2cff_3f0c = 0;
  DAT_2cff_3f0a = 0;
  for (iVar2 = 0; iVar2 < 0x32; iVar2 = iVar2 + 1) {
    ((undefined2 *)&DAT_2cff_422c)[iVar2 * 2] = 0xffff;
    ((undefined2 *)&DAT_2cff_422a)[iVar2 * 2] = 0xffff;
    ((undefined2 *)&DAT_2cff_3ea6)[iVar2] = 0;
    ((undefined2 *)&DAT_2cff_3e42)[iVar2] = 0;
    ((undefined2 *)&DAT_2cff_3f12)[iVar2 * 2] = 0;
    ((undefined2 *)&DAT_2cff_3f10)[iVar2 * 2] = 0;
  }
  for (iVar2 = 0; iVar2 < 0x80; iVar2 = iVar2 + 1) {
    ((undefined2 *)&DAT_2cff_3fdc)[iVar2 * 2] = 0;
    ((undefined2 *)&DAT_2cff_3fda)[iVar2 * 2] = 0;
  }
  DAT_2cff_3f0e = 0;
  DAT_2cff_42f4 = 0;
  iVar2 = 0;
  do {
    if (5 < iVar2) {
LAB_1581_00d5:
      if (*param_1 == '\0') {
        DAT_2cff_3f0e = 0;
      }
      else {
        DAT_2cff_439c = FUN_2b4b_000c((char *)param_1,param_1._2_2_,0x8001);
        if (DAT_2cff_439c == -1) {
          DAT_2cff_3f0e = 0;
        }
        else {
          FUN_2b58_000d(DAT_2cff_439c,(undefined2 *)&DAT_2cff_422a,0x2cff,200);
          FUN_2b58_000d(DAT_2cff_439c,(undefined2 *)&DAT_2cff_3ea6,0x2cff,100);
          FUN_2b58_000d(DAT_2cff_439c,(undefined2 *)&DAT_2cff_3e42,0x2cff,100);
          FUN_2b58_000d(DAT_2cff_439c,0x42f6,0x2cff,0xa0);
          FUN_2b58_000d(DAT_2cff_439c,0x41da,0x2cff,0x50);
        }
      }
      return;
    }
    for (iVar3 = 0; iVar3 < 4; iVar3 = iVar3 + 1) {
      DAT_2cff_0bf6 = *(undefined2 *)(iVar2 * 2 + 0xbd2);
      _DAT_2cff_0bf8 = *(undefined2 *)(iVar3 * 2 + 0xbde);
      iVar1 = FUN_15ed_04da();
      if (iVar1 != 0) {
        iVar1 = FUN_15ed_0338();
        if (iVar1 == 0) {
          DAT_2cff_3f0e = 1;
          DAT_2cff_42f4 = 1;
          goto LAB_1581_00d5;
        }
      }
    }
    iVar2 = iVar2 + 1;
  } while( true );
}


