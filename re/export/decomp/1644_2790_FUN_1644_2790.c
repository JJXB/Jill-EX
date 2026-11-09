// FUN_1644_2790 @ 1644:2790

uint __cdecl16far FUN_1644_2790(int param_1,uint param_2,int param_3)

{
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) / 2 + 0xe06
                            ,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      FUN_26f4_1a13(1);
      FUN_1581_0178(3,6);
      FUN_26f4_1863(param_1);
      if (DAT_2cff_0c24 != 0) {
        FUN_22ab_05b4(0xe73,0x2cff,2);
        DAT_2cff_0c24 = 0;
      }
      param_2 = 1;
    }
    else {
      param_2 = 0;
    }
  }
  else if (param_2 == 2) {
    *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) =
         *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + 1U & 7;
    param_2 = (uint)((*(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) & 1) == 0);
  }
  return param_2;
}


