// FUN_1a41_2069 @ 1a41:2069

int __cdecl16far FUN_1a41_2069(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  undefined2 unaff_SS;
  int local_42 [32];
  
  iVar3 = DAT_2cff_bb9a * 0x100;
  FUN_1000_05d8(0xfb4,0x2cff,local_42,unaff_SS);
  if (param_2 == 0) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 1) {
      iVar2 = 0;
    }
    else {
      iVar2 = 3;
    }
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            iVar3 + *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) +
                                    iVar2 + 4,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      param_2 = FUN_26f4_25b8(param_1);
    }
  }
  else if (param_2 == 2) {
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f);
    *piVar1 = *piVar1 + 1;
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) < 0xa1) {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
      *piVar1 = *piVar1 + 1;
      if (2 < *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 0;
      }
      FUN_26f4_06b4(param_1,(int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f,0x2cff,
                    (int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f,0x2cff);
      iVar3 = FUN_2ca6_0015();
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
           ((iVar3 % 2 + *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) * 2) *
           local_42[*(uint *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) & 0x1f]) / 2;
      iVar3 = FUN_2ca6_0015();
      *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) =
           ((iVar3 % 2 + *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) * 2) *
           local_42[*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) + 0x10U & 0x1f]) /
           2;
      FUN_26f4_1d73(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                    *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                    *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
      param_2 = 1;
    }
    else {
      FUN_26f4_1863(param_1);
      param_2 = 1;
    }
  }
  return param_2;
}


