// FUN_1a41_165a @ 1a41:165a

int __cdecl16far FUN_1a41_165a(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bb92 * 0x100 +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 2) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) {
      param_2 = 0;
    }
    else {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      *piVar1 = *piVar1 + 2;
      if (0x10 < *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0x10;
      }
      iVar2 = FUN_26f4_1d73(param_1,*(undefined2 *)
                                     ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
      if (iVar2 == 0) {
        *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) =
             2 - *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) == 0) {
          param_2 = 0;
        }
        else {
          FUN_1581_0178(2,0x1c);
          param_2 = 1;
        }
      }
      else {
        param_2 = 1;
      }
    }
  }
  else if (param_2 == 3) {
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 1;
    param_2 = 1;
  }
  return param_2;
}


