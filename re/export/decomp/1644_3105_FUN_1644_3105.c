// FUN_1644_3105 @ 1644:3105

int __cdecl16far FUN_1644_3105(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bb62 * 0x100 +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) / 2,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      FUN_26f4_25b8(param_1);
      param_2 = FUN_1a41_0930(0);
    }
  }
  else if (param_2 == 2) {
    iVar2 = FUN_26f4_1da8(param_1);
    if (iVar2 == 0) {
      FUN_26f4_1863(param_1);
      param_2 = 1;
    }
    else {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
      *piVar1 = *piVar1 + 1;
      if (7 < *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 0;
      }
      iVar2 = FUN_26f4_1d73(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                    *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                            *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
      if (iVar2 == 0) {
        FUN_26f4_1863(param_1);
      }
      param_2 = 1;
    }
  }
  return param_2;
}


