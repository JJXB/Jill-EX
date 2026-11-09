// FUN_1a41_3834 @ 1a41:3834

int __cdecl16far FUN_1a41_3834(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bbb2 * 0x100 +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + 6,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 2) {
    iVar2 = FUN_2ca6_0015();
    if (iVar2 % 0xf == 0) {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
      *piVar1 = *piVar1 + 1;
    }
    if ((*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) < 3) &&
       (iVar2 = FUN_26f4_1da8(param_1), iVar2 != 0)) {
      iVar2 = FUN_2ca6_0015((*(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) -
                            *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)) + -1);
      iVar2 = FUN_26f4_2605(param_1,iVar2 % 3 +
                                    *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                    -1);
      if (iVar2 == 0) {
        FUN_26f4_1863(param_1);
      }
    }
    else {
      FUN_26f4_1863(param_1);
    }
    param_2 = 1;
  }
  return param_2;
}


