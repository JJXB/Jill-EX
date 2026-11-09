// FUN_1a41_48f9 @ 1a41:48f9

int __cdecl16far FUN_1a41_48f9(int param_1,int param_2,int param_3)

{
  int iVar1;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bbc0 * 0x100 +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) / 2,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      if ((((undefined2 *)&DAT_2cff_be2c)[DAT_2cff_96b9] & 2) == 0) {
        iVar1 = FUN_26f4_1d73(0,DAT_2cff_96ad + -8,DAT_2cff_96af);
        if (iVar1 == 0) {
          FUN_26f4_1d73(0,DAT_2cff_96ad + 8,DAT_2cff_96af);
        }
        DAT_2cff_96b9 = 0;
        DAT_2cff_96bb = 0;
      }
      param_2 = FUN_26f4_25b8(param_1);
    }
  }
  else if (param_2 == 2) {
    *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) =
         *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + 1U & 7;
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) == 0) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 2;
    }
    iVar1 = FUN_26f4_232e(param_1,*(undefined2 *)
                                   ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                          *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                          *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f),4);
    if (iVar1 == 0) {
      FUN_26f4_1ad4(param_1,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                    *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
    }
    else {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) =
           -*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
    }
    param_2 = 1;
  }
  return param_2;
}


