// FUN_1a41_269f @ 1a41:269f

int __cdecl16far FUN_1a41_269f(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  undefined2 unaff_SS;
  int local_16 [4];
  int local_e [4];
  int local_6 [2];
  
  FUN_1000_05d8(0xff4,0x2cff,local_16,unaff_SS);
  FUN_1000_05d8(0xffc,0x2cff,local_e,unaff_SS);
  iVar2 = DAT_2cff_bb9e * 0x100;
  if (param_2 == 0) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 1) {
      FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                    local_16[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)] + iVar2 +
                    8,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                              local_e[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)]
                              + iVar2 + 8,
                              *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) + 0x20,
                              *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
    }
    else {
      FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                    local_16[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)] + iVar2,
                    *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) + 0x20,
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                              local_e[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)]
                              + iVar2,*(undefined2 *)
                                       ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                              *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
    }
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      FUN_26f4_25b8(param_1);
      param_2 = 1;
    }
    else {
      param_2 = (int)(char)((undefined1 *)&DAT_2cff_96ac)[param_3 * 0x1f];
      if (((*(uint *)(param_2 * 8 + -0x7c12) & 0x4000) != 0) &&
         (param_2 = param_1 * 0x1f, *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_2) == 0)) {
        FUN_26f4_012d(param_1);
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 0x10;
        FUN_26f4_06b4(param_1,local_6);
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) * local_6[0] < 1) {
          param_2 = FUN_26f4_0091(param_1);
        }
        else {
          param_2 = -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
          *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = param_2;
        }
      }
    }
  }
  else if (param_2 == 2) {
    if (0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f)) {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f);
      *piVar1 = *piVar1 + -1;
    }
    *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) =
         *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + 1U & 3;
    iVar2 = FUN_26f4_1f35(param_1,*(undefined2 *)
                                   ((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),0);
    if (iVar2 == 0) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
           -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    }
    param_2 = 1;
  }
  return param_2;
}


