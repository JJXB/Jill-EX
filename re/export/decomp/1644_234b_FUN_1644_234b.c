// FUN_1644_234b @ 1644:234b

int __cdecl16far FUN_1644_234b(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  
  if (param_2 == 0) {
    iVar2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                          DAT_2cff_bb5e * 0x100 +
                          (uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f)) *
                          5 + *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f),
                          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
    return iVar2;
  }
  iVar2 = 1;
  if (param_2 != 1) {
    if (param_2 != 2) {
      return param_2;
    }
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) == 0) {
      *(uint *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) =
           *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) + 1U & 3;
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 1;
      iVar2 = FUN_26f4_1f35(param_1,*(undefined2 *)
                                     ((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),0);
      if (iVar2 == 0) {
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
             -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
      }
      return 1;
    }
    iVar2 = param_1 * 0x1f;
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + iVar2);
    *piVar1 = *piVar1 + -1;
  }
  if (param_3 == 0) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96c9 + param_1 * 0x1f) == 0) {
      FUN_22ab_05b4(DAT_2cff_96c3,DAT_2cff_96c5,7);
    }
    iVar2 = 0;
    DAT_2cff_96c9 = 3;
  }
  return iVar2;
}


