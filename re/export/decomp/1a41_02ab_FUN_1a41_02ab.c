// FUN_1a41_02ab @ 1a41:02ab

int __cdecl16far FUN_1a41_02ab(int param_1,int param_2,int param_3)

{
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bb7e * 0x100 +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      if (DAT_2cff_3c78 != 0) {
        DAT_2cff_96b3 = 0;
      }
      if (DAT_2cff_0ecc != 0) {
        DAT_2cff_0ecc = 0;
        FUN_22ab_05b4((char *)s_Press_UP_DOWN_to_toggle_switch_2cff_110e,0x2cff,2);
      }
      if ((DAT_2cff_3c78 < 0) && (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 1)
         ) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 0;
        FUN_1581_0178(2,0x17);
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) == 1) {
          FUN_26f4_20ef(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f),3,
                        param_1);
        }
        else {
          FUN_26f4_20ef(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f),5,
                        param_1);
        }
      }
      else if ((0 < DAT_2cff_3c78) &&
              (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 1;
        FUN_1581_0178(2,0x18);
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) == 1) {
          FUN_26f4_20ef(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f),3,
                        param_1);
        }
        else {
          FUN_26f4_20ef(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f),4,
                        param_1);
        }
      }
    }
    param_2 = 1;
  }
  else if (param_2 == 2) {
    param_2 = 0;
  }
  return param_2;
}


