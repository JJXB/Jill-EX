// FUN_1a41_17ee @ 1a41:17ee

int __cdecl16far FUN_1a41_17ee(int param_1,int param_2,int param_3)

{
  int iVar1;
  undefined2 unaff_SS;
  int local_18;
  int local_16;
  int local_12 [4];
  int local_a [4];
  
  local_18 = DAT_2cff_bb94 * 0x100;
  local_16 = 0;
  FUN_1000_05d8(0xf78,0x2cff,local_12,unaff_SS);
  FUN_1000_05d8(0xf80,0x2cff,local_a,unaff_SS);
  if (param_2 == 0) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) {
      local_16 = 6;
      iVar1 = 2;
    }
    else {
      iVar1 = local_12[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) / 2];
      local_18 = local_18 +
                 local_a[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) / 2] +
                 (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 0) * 3;
    }
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,local_18,
                            *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) + local_16,
                            *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + iVar1);
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      param_2 = FUN_26f4_25b8(param_1);
    }
  }
  else if (param_2 == 2) {
    *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) =
         *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + 1U & 7;
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) {
      iVar1 = FUN_2ca6_0015();
      if (iVar1 % 0x28 != 0) {
        return 0;
      }
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 1;
      iVar1 = FUN_2ca6_0015();
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = (iVar1 % 2) * 8 + -4;
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 2;
    }
    else {
      iVar1 = FUN_26f4_1c89(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                    *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                            *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
      if (iVar1 == 1) {
        iVar1 = FUN_2ca6_0015();
        if (iVar1 % 0x23 == 0) {
          *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
               -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
        }
        iVar1 = FUN_2ca6_0015();
        if (iVar1 % 0x14 == 0) {
          *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) =
               -*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
        }
      }
      else if (iVar1 == 2) {
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
             -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
      }
      else {
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
             -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
        iVar1 = FUN_2ca6_0015();
        if ((iVar1 % 3 == 0) && (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 0))
        {
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 0;
          FUN_26f4_1c89(param_1,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                        *(uint *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) & 0xfff0);
        }
        else {
          *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) =
               -*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
        }
      }
    }
    param_2 = 1;
  }
  return param_2;
}


