// FUN_1644_28a4 @ 1644:28a4

int __cdecl16far FUN_1644_28a4(int param_1,int param_2,int param_3)

{
  undefined2 uVar1;
  
  if (param_2 == 0) {
    if (DAT_2cff_120e != 0) {
      param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,0x140,
                              *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) + 4,
                              *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 4);
    }
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == -1) {
        uVar1 = 4;
      }
      else if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 1) {
        uVar1 = 5;
      }
      else {
        uVar1 = 3;
      }
      FUN_26f4_20ef(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f),uVar1,
                    param_1);
    }
    param_2 = 1;
  }
  else if (param_2 == 2) {
    param_2 = 0;
  }
  return param_2;
}


