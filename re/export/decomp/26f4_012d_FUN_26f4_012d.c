// FUN_26f4_012d @ 26f4:012d

void __cdecl16far FUN_26f4_012d(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int local_4;
  
  iVar1 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f);
  iVar2 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f);
  iVar3 = *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f);
  iVar4 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f);
  iVar5 = *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f);
  for (local_4 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) >> 4;
      local_4 < iVar4 + iVar5 + 0xf >> 4; local_4 = local_4 + 1) {
    for (iVar6 = iVar1 >> 4; iVar6 < iVar2 + iVar3 + 0xf >> 4; iVar6 = iVar6 + 1) {
      ((undefined2 *)&DAT_2cff_43a6)[iVar6 * 0x40 + local_4] =
           ((undefined2 *)&DAT_2cff_43a6)[iVar6 * 0x40 + local_4] | 0xc000;
    }
  }
  return;
}


