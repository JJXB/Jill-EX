// FUN_1644_3da3 @ 1644:3da3

int __cdecl16far FUN_1644_3da3(int param_1,int param_2)

{
  int iVar1;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,0xe10,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 2) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) {
      param_2 = 0;
    }
    else {
      iVar1 = FUN_26f4_1d73(param_1,*(undefined2 *)
                                     ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 4);
      if (iVar1 == 0) {
        FUN_1581_0178(3,0xe);
        ((undefined2 *)&DAT_2cff_43a6)
        [(*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) / 0x10) * 0x40 +
         *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) / 0x10] =
             *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) | 0xc000;
        FUN_26f4_1863(param_1);
      }
      else if ((*(uint *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) & 0xf) == 0) {
        ((undefined2 *)&DAT_2cff_43a6)
        [(*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) / 0x10) * 0x40 +
         *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) / 0x10 + -1] =
             *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) | 0xc000;
      }
      param_2 = 1;
    }
  }
  else if (param_2 == 3) {
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 1;
    *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) =
         ((undefined2 *)&DAT_2cff_43a6)
         [(*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) >> 4) * 0x40 +
          (*(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) >> 4) + -1] & 0x3fff;
    param_2 = FUN_1581_0178(3,0xd);
  }
  return param_2;
}


