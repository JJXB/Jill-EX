// FUN_1a41_45ed @ 1a41:45ed

int __cdecl16far FUN_1a41_45ed(int param_1,int param_2)

{
  int iVar1;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bbbc * 0x100 +
                            (uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f))
                            + 0x15,*(undefined2 *)
                                    ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 2) {
    iVar1 = FUN_26f4_1da8(param_1);
    if (iVar1 == 0) {
      FUN_26f4_1863(param_1);
      param_2 = 0;
    }
    else {
      iVar1 = FUN_26f4_1d73(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                    *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      if (iVar1 == 0) {
        FUN_26f4_1863(param_1);
      }
      param_2 = 1;
    }
  }
  return param_2;
}


