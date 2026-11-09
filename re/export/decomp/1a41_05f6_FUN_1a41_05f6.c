// FUN_1a41_05f6 @ 1a41:05f6

int __cdecl16far FUN_1a41_05f6(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + 0xe18,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      param_2 = FUN_26f4_25b8(param_1);
    }
  }
  else if (param_2 == 2) {
    iVar2 = FUN_26f4_2227(param_1,*(undefined2 *)
                                   ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                          *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 1,3);
    if (iVar2 == 3) {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      *piVar1 = *piVar1 + 2;
      if (0xc < *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0xc;
      }
      iVar2 = FUN_26f4_1c89(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                    *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                            *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
      if (iVar2 != 1) {
        FUN_26f4_1c89(param_1,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                      ((*(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                       *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f)) - 1U & 0xfff0)
                      + (0x10 - DAT_2cff_b786));
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = 0;
      }
    }
    else {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) != 0) {
        FUN_1581_0178(2,0x2d);
      }
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0;
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) == 0) {
        FUN_26f4_06b4(param_1,(int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f,0x2cff,
                      (int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f,0x2cff);
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
             *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) << 2;
      }
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 1) {
        iVar2 = -1;
      }
      else {
        iVar2 = 1;
      }
      *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) =
           *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + iVar2 & 3;
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0;
      iVar2 = FUN_26f4_1c89(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                    *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      if (iVar2 != 1) {
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
             -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
      }
    }
    param_2 = 1;
  }
  return param_2;
}


