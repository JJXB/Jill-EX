// FUN_1069_0233 @ 1069:0233

uint __cdecl16far FUN_1069_0233(int *param_1,int param_2)

{
  uint uVar1;
  byte bVar2;
  
  uVar1 = **(uint **)param_1;
  bVar2 = (byte)DAT_2cff_2fac & 0x1f;
  DAT_2cff_2fac = DAT_2cff_2fac + param_2;
  if (7 < DAT_2cff_2fac) {
    DAT_2cff_2fac = DAT_2cff_2fac + -8;
    *param_1 = *param_1 + 1;
  }
  return uVar1 >> bVar2 & (1 << ((byte)param_2 & 0x1f)) - 1U;
}


