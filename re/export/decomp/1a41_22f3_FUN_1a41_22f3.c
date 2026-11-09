// FUN_1a41_22f3 @ 1a41:22f3

int __cdecl16far FUN_1a41_22f3(int param_1,int param_2,int param_3)

{
  int iVar1;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) +
                            DAT_2cff_bb9c * 0x100,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      param_2 = FUN_26f4_25b8(param_1);
    }
  }
  else if (param_2 == 2) {
    *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) =
         *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + 1U & 3;
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) == 0) {
        iVar1 = FUN_2ca6_0015();
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = (iVar1 % 2) * 8 + -4;
      }
      if ((((*(uint *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) & 0xf) == 0) &&
          (iVar1 = FUN_26f4_2227(param_1,*(undefined2 *)
                                          ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                                 *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f)
                                 ,4), iVar1 == 0)) && (iVar1 = FUN_2ca6_0015(), iVar1 % 2 == 0)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 1;
      }
      if ((*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) &&
         (iVar1 = FUN_26f4_1f35(param_1,*(undefined2 *)
                                         ((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),0),
         iVar1 == 0)) {
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
             -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
      }
    }
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 1) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) == 0) {
        iVar1 = FUN_2ca6_0015();
        *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = (iVar1 % 2) * 4 + -2;
      }
      iVar1 = FUN_26f4_1d73(param_1,*(undefined2 *)
                                     ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
      if (iVar1 == 0) {
        *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) =
             -*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
        FUN_26f4_1d73(param_1,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                      *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                      *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
      }
      if ((*(uint *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) & 0xf) == 0) {
        iVar1 = FUN_26f4_1f35(param_1,*(undefined2 *)
                                       ((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),0);
        if (iVar1 == 0) {
          iVar1 = FUN_26f4_1f35(param_1,-*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f
                                                 ),0);
          if (iVar1 != 0) {
            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 0;
            *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
                 -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
          }
        }
        else {
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 0;
        }
      }
    }
    param_2 = 1;
  }
  return param_2;
}


