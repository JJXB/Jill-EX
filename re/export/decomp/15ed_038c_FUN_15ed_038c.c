// FUN_15ed_038c @ 15ed:038c

bool __cdecl16far FUN_15ed_038c(undefined2 param_1,undefined2 param_2,uint param_3)

{
  byte bVar1;
  undefined1 extraout_DL;
  bool bVar2;
  
  bVar2 = DAT_2cff_0bf9 == 0;
  if (bVar2) {
    DAT_2cff_0bf9 = 1;
    FUN_15ed_0034();
    FUN_15ed_0034();
    FUN_15ed_01a8();
    bVar1 = in(0x21);
    out(0x21,bVar1 & ~('\x01' << (DAT_2cff_0bf8 & 0x1f)));
    DAT_2cff_0c10 = FUN_15ed_0194();
    DAT_2cff_0c15 = param_3;
    DAT_2cff_0c17 = 0;
    DAT_2cff_0c19 = DAT_2cff_0c10 + param_3 + -1;
    DAT_2cff_0c14 = CARRY2(DAT_2cff_0c10,param_3) - (DAT_2cff_0c10 + param_3 == 0);
    DAT_2cff_0c0f = extraout_DL;
    FUN_15ed_0264();
  }
  return !bVar2;
}


