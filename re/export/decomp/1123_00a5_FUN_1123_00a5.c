// FUN_1123_00a5 @ 1123:00a5

void __cdecl16far FUN_1123_00a5(int param_1,int param_2,int *param_3,byte *param_4)

{
  uint uVar1;
  uint uVar2;
  
  uVar1 = param_2 * 0x50;
  uVar2 = DAT_2cff_3a49 + uVar1;
  ((int *)param_3)[1] =
       ((int)DAT_2cff_3a49 >> 0xf) + ((int)uVar1 >> 0xf) + (uint)CARRY2(DAT_2cff_3a49,uVar1) +
       (param_1 >> 0xf) + (uint)CARRY2(uVar2,param_1 >> 2) + -0x6000;
  *param_3 = uVar2 + (param_1 >> 2);
  *param_4 = (byte)param_1 & 3;
  return;
}


