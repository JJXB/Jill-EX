// FUN_1644_1e50 @ 1644:1e50

int __cdecl16far FUN_1644_1e50(int param_1,int param_2,int param_3)

{
  uint *puVar1;
  int *piVar2;
  int iVar3;
  
  if (param_2 == 0) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 1) {
      iVar3 = 6 - *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f);
    }
    else {
      iVar3 = *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f);
    }
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,DAT_2cff_bb54 * 0x100 + iVar3,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      param_2 = FUN_26f4_25b8(param_1);
    }
  }
  else if (param_2 == 2) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) {
      iVar3 = FUN_26f4_1f35(param_1,*(undefined2 *)
                                     ((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),0);
      if (iVar3 == 0) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 1;
      }
    }
    else {
      puVar1 = (uint *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f);
      *puVar1 = *puVar1 ^ 1;
      if ((*(uint *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) & 1) != 0) {
        return 0;
      }
      piVar2 = (int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f);
      *piVar2 = *piVar2 + 1;
      if (5 < *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 0;
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
             -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
      }
    }
    param_2 = 1;
  }
  return param_2;
}


