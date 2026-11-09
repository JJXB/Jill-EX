// FUN_1123_005d @ 1123:005d

void __cdecl16far FUN_1123_005d(int param_1,int param_2,int *param_3,byte *param_4)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  
  uVar1 = param_2 * 0x28;
  uVar2 = param_1 / 8;
  iVar3 = uVar1 + uVar2 + DAT_2cff_3a49;
  ((int *)param_3)[1] =
       ((int)uVar1 >> 0xf) + ((int)uVar2 >> 0xf) + (uint)CARRY2(uVar1,uVar2) +
       ((int)DAT_2cff_3a49 >> 0xf) + (uint)CARRY2(uVar1 + uVar2,DAT_2cff_3a49) + -0x6000;
  *param_3 = iVar3;
  *param_4 = (byte)param_1 & 7;
  return;
}


