// FUN_1a41_09a8 @ 1a41:09a8

void __cdecl16far FUN_1a41_09a8(undefined2 param_1,undefined2 param_2,int param_3)

{
  int iVar1;
  int iVar2;
  
  for (iVar2 = 0; iVar2 < param_3; iVar2 = iVar2 + 1) {
    FUN_26f4_1892(0x24,param_1,param_2);
    iVar1 = FUN_2ca6_0015();
    *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + (DAT_2cff_bb1c + -1) * 0x1f) = iVar1 % 7 + -3;
    iVar1 = FUN_2ca6_0015();
    *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + (DAT_2cff_bb1c + -1) * 0x1f) = iVar1 % 0xb + -8;
    iVar1 = FUN_2ca6_0015();
    *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + (DAT_2cff_bb1c + -1) * 0x1f) = iVar1 % 3;
  }
  return;
}


