// FUN_1a41_04b2 @ 1a41:04b2

uint __cdecl16far FUN_1a41_04b2(int param_1,uint param_2,int param_3)

{
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bb80 * 0x100 +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) / 2 + 4,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      if (DAT_2cff_0ed2 != 0) {
        DAT_2cff_0ed2 = 0;
        FUN_22ab_05b4((char *)s_USE_GEMS_TO_OPEN_DOORS_ON_THE_MA_2cff_112d,0x2cff,2);
      }
      FUN_26f4_1a13(3);
      FUN_1581_0178(3,0x10);
      FUN_26f4_1fb2(DAT_2cff_bc0a,
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      param_2 = FUN_26f4_1863(param_1);
    }
  }
  else if (param_2 == 2) {
    *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) =
         *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + 1U & 7;
    param_2 = (uint)((*(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) & 1) == 0);
  }
  return param_2;
}


