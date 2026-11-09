// FUN_1644_1789 @ 1644:1789

int __cdecl16far FUN_1644_1789(int param_1,int param_2,int param_3)

{
  int iVar1;
  undefined2 unaff_SS;
  int local_1a [4];
  int local_12 [8];
  
  FUN_1000_05d8(0xc2a,0x2cff,local_1a,unaff_SS);
  FUN_1000_05d8(0xc32,0x2cff,local_12,unaff_SS);
  if (param_2 == 0) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 1) {
      iVar1 = 0;
    }
    else {
      iVar1 = 4;
    }
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            local_1a[*(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) &
                                     3] + DAT_2cff_bb48 * 0x100 + iVar1,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      FUN_1a41_09a8(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f),4);
      FUN_26f4_05e7(1,0);
      param_2 = FUN_26f4_1863(param_1);
    }
  }
  else if (param_2 == 2) {
    iVar1 = FUN_26f4_1d73(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                  *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                          local_12[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)] +
                          *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
    if (iVar1 == 0) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
           -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
      FUN_1581_0178(1,0xf);
    }
    *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) =
         *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + 1U & 7;
    param_2 = 1;
  }
  return param_2;
}


