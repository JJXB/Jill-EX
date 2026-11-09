// FUN_1a41_3029 @ 1a41:3029

int __cdecl16far FUN_1a41_3029(int param_1,int param_2)

{
  int iVar1;
  undefined2 unaff_SS;
  int local_1a [8];
  int local_a [4];
  
  iVar1 = DAT_2cff_bba4 * 0x100;
  FUN_1000_05d8(0x1004,0x2cff,local_1a,unaff_SS);
  FUN_1000_05d8(0x1014,0x2cff,local_a,unaff_SS);
  if (param_2 == 0) {
    FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                  local_a[*(int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) / 2 & 3] +
                  iVar1,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
    param_2 = param_1 * 0x1f;
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_2) != 0) {
      FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                    local_1a[*(int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f)] + iVar1,
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 5);
      param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                              local_1a[*(int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f)
                                       + 4U & 7] + iVar1,
                              *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) + 10,
                              *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 6);
    }
  }
  else if (param_2 == 2) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) {
      param_2 = 0;
    }
    else {
      *(uint *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) =
           *(int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) + 1U & 7;
      param_2 = 1;
    }
  }
  else if (param_2 == 3) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 1;
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) = 0;
      FUN_1581_0178(4,0x21);
      param_2 = 1;
    }
    else {
      param_2 = 0;
    }
  }
  return param_2;
}


