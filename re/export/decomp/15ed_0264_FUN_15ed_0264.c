// FUN_15ed_0264 @ 15ed:0264

void __cdecl16near FUN_15ed_0264(void)

{
  int iVar1;
  uint uVar2;
  bool bVar3;
  
  iVar1 = -1;
  if (DAT_2cff_0c14 == '\0') {
    DAT_2cff_0c14 = '\x01';
    iVar1 = DAT_2cff_0c19;
  }
  DAT_2cff_0c12 = iVar1 - DAT_2cff_0c10;
  uVar2 = DAT_2cff_0c12 + 1;
  if (uVar2 == 0) {
    DAT_2cff_0c17 = DAT_2cff_0c17 + -1;
  }
  else {
    bVar3 = DAT_2cff_0c15 < uVar2;
    DAT_2cff_0c15 = DAT_2cff_0c15 - uVar2;
    DAT_2cff_0c17 = DAT_2cff_0c17 - (uint)bVar3;
  }
  FUN_15ed_016b();
  DAT_2cff_0c14 = DAT_2cff_0c14 + -1;
  DAT_2cff_0c0f = DAT_2cff_0c0f + '\x01';
  DAT_2cff_0c10 = 0;
  FUN_15ed_0034();
  FUN_15ed_0034();
  FUN_15ed_0034();
  return;
}


