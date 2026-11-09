// FUN_1644_2d43 @ 1644:2d43

int __cdecl16far FUN_1644_2d43(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  undefined2 unaff_SS;
  int local_a [4];
  
  FUN_1000_05d8(0xc6e,0x2cff,local_a,unaff_SS);
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bb60 * 0x100 +
                            (uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f))
                            * 3 + local_a[*(int *)((int)(undefined2 *)&DAT_2cff_96bf +
                                                  param_1 * 0x1f) / 4],
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      param_2 = FUN_26f4_25b8(param_1);
    }
  }
  else if (param_2 == 2) {
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
    *piVar1 = *piVar1 + 1;
    if (0xf < *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 0;
    }
    if ((*(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) & 1) == 0) {
      iVar2 = FUN_2ca6_0015();
      if (iVar2 % 0x1e == 0) {
        FUN_26f4_06b4(param_1,(int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f,0x2cff,
                      (int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f,0x2cff);
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0;
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
             *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) << 2;
      }
      iVar2 = FUN_26f4_1f35(param_1,*(undefined2 *)
                                     ((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),0);
      if (iVar2 == 0) {
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
             -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
      }
      else {
        FUN_1581_0178(1,0x11);
      }
      param_2 = 1;
    }
    else {
      param_2 = 0;
    }
  }
  return param_2;
}


