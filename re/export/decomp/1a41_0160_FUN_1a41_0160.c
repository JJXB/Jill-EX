// FUN_1a41_0160 @ 1a41:0160

uint __cdecl16far FUN_1a41_0160(int param_1,uint param_2,int param_3)

{
  int *piVar1;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bb7c * 0x100 +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) / 2 +
                            (uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f))
                            * 6,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if ((param_3 == 0) &&
       (param_2 = param_1 * 0x1f, *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_2) != 1)) {
      FUN_26f4_05e7(2,0);
      FUN_1a41_0930(0);
      param_2 = param_1 * 0x1f;
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_2) = 1;
    }
  }
  else if (param_2 == 2) {
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
    *piVar1 = *piVar1 + 1;
    if ((0xb < *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)) ||
       (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) < 0)) {
      FUN_26f4_1863(param_1);
    }
    param_2 = (uint)((*(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) & 1) == 0);
  }
  return param_2;
}


