// FUN_1123_0002 @ 1123:0002

void __cdecl16far FUN_1123_0002(int param_1,uint param_2,int *param_3,char *param_4)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  
  uVar1 = param_1 / 4;
  uVar2 = (param_2 & 1) * 0x2000;
  uVar4 = uVar1 + uVar2;
  uVar3 = ((int)param_2 / 2) * 0x50;
  ((int *)param_3)[1] =
       ((int)uVar1 >> 0xf) + (uint)CARRY2(uVar1,uVar2) + ((int)uVar3 >> 0xf) +
       (uint)CARRY2(uVar4,uVar3) + -0x4800;
  *param_3 = uVar4 + uVar3;
  *param_4 = ((byte)param_1 & 3) << 1;
  return;
}


