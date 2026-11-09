// FUN_1a41_3957 @ 1a41:3957

int __cdecl16far FUN_1a41_3957(int param_1,int param_2,int param_3)

{
  int *piVar1;
  uint uVar2;
  int iVar3;
  undefined2 uVar4;
  uint uVar5;
  undefined2 unaff_SS;
  int local_16 [10];
  
  FUN_1000_05d8(0x101c,0x2cff,local_16,unaff_SS);
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            local_16[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)] +
                            DAT_2cff_bbb4 * 0x100,
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
        FUN_1a41_09a8(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f),0x14);
        param_2 = FUN_26f4_1863(param_1);
      }
    }
  }
  else if (param_2 == 2) {
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
    *piVar1 = *piVar1 + 1;
    if (9 < *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 0;
    }
    iVar3 = FUN_2ca6_0015();
    uVar2 = *(uint *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    uVar5 = (int)uVar2 >> 0xf;
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    *piVar1 = *piVar1 + (iVar3 % 3 + -1) * (((int)((uVar2 ^ uVar5) - uVar5) < 3) + 1);
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 7) {
      iVar3 = *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    }
    else {
      iVar3 = 6;
    }
    if (iVar3 < -6) {
      uVar4 = 0xfffa;
    }
    else if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 7) {
      uVar4 = *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    }
    else {
      uVar4 = 6;
    }
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = uVar4;
    iVar3 = FUN_2ca6_0015();
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
    *piVar1 = *piVar1 + (iVar3 % 3 + -1) *
                        ((*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 2) + 1);
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 7) {
      iVar3 = *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
    }
    else {
      iVar3 = 6;
    }
    if (iVar3 < -6) {
      uVar4 = 0xfffa;
    }
    else if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 7) {
      uVar4 = *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
    }
    else {
      uVar4 = 6;
    }
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = uVar4;
    iVar3 = FUN_26f4_2605(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                  *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
    if (iVar3 == 0) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
           -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    }
    iVar3 = FUN_26f4_2605(param_1,*(undefined2 *)
                                   ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                          *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                          *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
    if (iVar3 == 0) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) =
           -*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
    }
    param_2 = 1;
  }
  return param_2;
}


