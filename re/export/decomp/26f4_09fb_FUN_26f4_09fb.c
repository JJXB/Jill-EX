// FUN_26f4_09fb @ 26f4:09fb

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_26f4_09fb(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  undefined2 uVar9;
  int local_1e;
  int local_1c;
  int local_18;
  int local_16;
  undefined2 local_14;
  
  local_1e = DAT_2cff_43a2 >> 4;
  iVar2 = DAT_2cff_43a4 >> 4;
  iVar3 = DAT_2cff_43a2 + DAT_2cff_96b5 + 0xf >> 4;
  iVar4 = DAT_2cff_43a4 + DAT_2cff_96b7 + 0xf >> 4;
  iVar5 = DAT_2cff_96ad >> 4;
  iVar6 = DAT_2cff_96af >> 4;
  iVar7 = DAT_2cff_96ad + DAT_2cff_96b5 + 0xf >> 4;
  iVar8 = DAT_2cff_96af + DAT_2cff_96b7 + 0xf >> 4;
  if (param_1 == 0) {
    if (local_1e < iVar5) {
      iVar5 = local_1e;
    }
    local_18 = iVar5 * 0x10 - *(int *)(DAT_2cff_b7f2 + 8);
    if (iVar7 < iVar3) {
      iVar7 = iVar3;
    }
    local_16 = iVar7 * 0x10 - *(int *)(DAT_2cff_b7f2 + 8);
    local_14 = *(undefined2 *)(DAT_2cff_b7f2 + 4);
    FUN_11f4_09c6(DAT_2cff_b7f2,DAT_2cff_b7f4,0,0,local_18,*(undefined2 *)(DAT_2cff_b7f2 + 6),0,
                  -param_2);
  }
  else if (param_2 == 0) {
    if (iVar2 < iVar6) {
      iVar6 = iVar2;
    }
    local_18 = iVar6 * 0x10 - *(int *)(DAT_2cff_b7f2 + 10);
    if (iVar8 < iVar4) {
      iVar8 = iVar4;
    }
    local_16 = iVar8 * 0x10 - *(int *)(DAT_2cff_b7f2 + 10);
    local_14 = *(undefined2 *)(DAT_2cff_b7f2 + 6);
    FUN_11f4_09c6(DAT_2cff_b7f2,DAT_2cff_b7f4,0,0,*(undefined2 *)(DAT_2cff_b7f2 + 4),local_18,
                  -param_1,0);
  }
  for (; local_1e < iVar3; local_1e = local_1e + 1) {
    for (local_1c = iVar2; local_1c < iVar4; local_1c = local_1c + 1) {
      FUN_22ab_140b(local_1e,local_1c);
      ((undefined2 *)&DAT_2cff_43a6)[local_1e * 0x40 + local_1c] =
           ((undefined2 *)&DAT_2cff_43a6)[local_1e * 0x40 + local_1c] & 0x7fff;
    }
  }
  iVar8 = (int)_DAT_2cff_b7f2;
  uVar9 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
  if (param_1 == 0) {
    FUN_11f4_09c6(iVar8,uVar9,local_18,0,local_16,*(undefined2 *)(iVar8 + 6),0,-param_2);
    piVar1 = (int *)((int)_DAT_2cff_b7f2 + 10);
    *piVar1 = *piVar1 + param_2;
    (*(code *)((undefined2 *)&DAT_2cff_b84e)[DAT_2cff_96ac * 2])(0x11f4,0,0,0);
    uVar9 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
    FUN_11f4_09c6((int)_DAT_2cff_b7f2,uVar9,local_16,0,local_14,
                  *(undefined2 *)((int)_DAT_2cff_b7f2 + 6),0,-param_2);
  }
  else if (param_2 == 0) {
    FUN_11f4_09c6(iVar8,uVar9,0,local_18,*(undefined2 *)(iVar8 + 4),local_16,-param_1,0);
    piVar1 = (int *)((int)_DAT_2cff_b7f2 + 8);
    *piVar1 = *piVar1 + param_1;
    (*(code *)((undefined2 *)&DAT_2cff_b84e)[DAT_2cff_96ac * 2])(0x11f4,0,0,0);
    uVar9 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
    FUN_11f4_09c6((int)_DAT_2cff_b7f2,uVar9,0,local_16,*(undefined2 *)((int)_DAT_2cff_b7f2 + 4),
                  local_14,-param_1,0);
  }
  else {
    FUN_11f4_0e4c(iVar8,uVar9,-param_1,-param_2);
    piVar1 = (int *)((int)_DAT_2cff_b7f2 + 8);
    *piVar1 = *piVar1 + param_1;
    piVar1 = (int *)((int)_DAT_2cff_b7f2 + 10);
    *piVar1 = *piVar1 + param_2;
    (*(code *)((undefined2 *)&DAT_2cff_b84e)[DAT_2cff_96ac * 2])(0x11f4,0,0,0);
  }
  FUN_26f4_0752(param_1,param_2,0x4000);
  return;
}


