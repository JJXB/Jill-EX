// FUN_1a41_29fa @ 1a41:29fa

int __cdecl16far FUN_1a41_29fa(int param_1,int param_2,int param_3)

{
  int *piVar1;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,DAT_2cff_bba0 * 0x100 + (DAT_2cff_be3a & 7),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      FUN_26f4_1fb2(2,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f);
      *piVar1 = *piVar1 + 1;
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) < 0xb) {
        param_2 = FUN_1581_0178(2,0x20);
      }
      else {
        FUN_1a41_09a8(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f),10);
        FUN_26f4_1863(param_1);
        param_2 = FUN_1581_0178(3,0x30);
      }
    }
  }
  else if (param_2 == 2) {
    param_2 = 1;
  }
  return param_2;
}


