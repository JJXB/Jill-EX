// FUN_1a41_3d03 @ 1a41:3d03

int __cdecl16far FUN_1a41_3d03(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  undefined2 unaff_SS;
  int local_a [4];
  
  FUN_1000_05d8(0x1030,0x2cff,local_a,unaff_SS);
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            local_a[*(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) &
                                    3] + DAT_2cff_bbb6 * 0x100 +
                            (uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f))
                            * 3 + 0xf,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      param_2 = FUN_26f4_25b8(param_1);
    }
    else {
      param_2 = (int)(char)((undefined1 *)&DAT_2cff_96ac)[param_3 * 0x1f];
      if ((((undefined2 *)&DAT_2cff_be3c)[param_2] & 0x4000) != 0) {
        param_2 = FUN_26f4_0091(param_1);
      }
    }
  }
  else if (param_2 == 2) {
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
    *piVar1 = *piVar1 + 1;
    iVar2 = FUN_2ca6_0015();
    if (iVar2 % 0x14 == 0) {
      iVar2 = FUN_2ca6_0015();
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = (iVar2 % 3) * 4 + -4;
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) == 0) {
        iVar2 = FUN_2ca6_0015();
        *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = (iVar2 % 2) * 4 + -2;
      }
      else {
        iVar2 = FUN_2ca6_0015();
        *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = (iVar2 % 3) * 2 + -2;
      }
    }
    iVar2 = FUN_26f4_2605(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                  *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
    if (iVar2 == 0) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
           -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    }
    iVar2 = FUN_26f4_2605(param_1,*(undefined2 *)
                                   ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                          *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                          *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
    if (iVar2 == 0) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) =
           -*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
    }
    iVar2 = FUN_2ca6_0015();
    if (iVar2 % 4 == 0) {
      FUN_26f4_1892(0x3a,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) + 6,
                    *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + -2);
    }
    param_2 = 1;
  }
  return param_2;
}


