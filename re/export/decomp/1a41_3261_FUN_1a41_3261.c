// FUN_1a41_3261 @ 1a41:3261

int __cdecl16far FUN_1a41_3261(int param_1,int param_2)

{
  int *piVar1;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            ((DAT_2cff_bba6 * 0x100 + 1) -
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f)) +
                            (uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f))
                            * 2,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f) == 0) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) =
           1 - *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f);
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 1) {
        FUN_26f4_20ef(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f),4,param_1)
        ;
        FUN_1581_0178(3,0x2c);
      }
      else {
        FUN_26f4_20ef(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f),5,param_1)
        ;
        FUN_1581_0178(3,0x2c);
      }
    }
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f) = 3;
    param_2 = 1;
  }
  else if (param_2 == 2) {
    if (0 < *(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f)) {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f);
      *piVar1 = *piVar1 + -1;
    }
    param_2 = 0;
  }
  return param_2;
}


