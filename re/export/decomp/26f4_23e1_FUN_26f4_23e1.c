// FUN_26f4_23e1 @ 26f4:23e1

void __cdecl16far FUN_26f4_23e1(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int local_c;
  
  if ((((undefined2 *)&DAT_2cff_be2c)[*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f)]
      & 2) == 0) {
    iVar1 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f);
    iVar2 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f);
    iVar3 = *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f);
    iVar4 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f);
    iVar5 = *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f);
    for (local_c = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) >> 4;
        local_c < iVar4 + iVar5 + 0xf >> 4; local_c = local_c + 1) {
      for (iVar6 = iVar1 >> 4; iVar6 < iVar2 + iVar3 + 0xf >> 4; iVar6 = iVar6 + 1) {
        if ((*(uint *)(((undefined2 *)&DAT_2cff_43a6)[iVar6 * 0x40 + local_c] * 8 + -0x7c12) & 8) !=
            0) {
          FUN_1f28_0006(iVar6,local_c,1);
        }
      }
    }
  }
  return;
}


