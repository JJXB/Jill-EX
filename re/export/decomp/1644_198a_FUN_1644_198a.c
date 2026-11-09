// FUN_1644_198a @ 1644:198a

int __cdecl16far FUN_1644_198a(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  undefined2 uVar3;
  int iVar4;
  undefined2 unaff_SS;
  int local_22 [8];
  int local_12 [8];
  
  FUN_1000_05d8(0xc42,0x2cff,local_22,unaff_SS);
  FUN_1000_05d8(0xc52,0x2cff,local_12,unaff_SS);
  if (param_2 == 0) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 0) {
      iVar4 = local_12[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)];
    }
    else {
      iVar4 = 0;
    }
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            local_22[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)] +
                            DAT_2cff_bb7a * 0x100 + iVar4,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      FUN_1a41_09a8(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f),4);
      FUN_26f4_05e7(1,0);
      FUN_1a41_0930(0);
      param_2 = FUN_26f4_1863(param_1);
    }
  }
  else if (param_2 == 2) {
    iVar4 = FUN_26f4_2227(param_1,*(undefined2 *)
                                   ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f),1);
    if (iVar4 == 0) {
      param_2 = 0;
    }
    else {
      iVar4 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f);
      iVar2 = *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
      uVar3 = *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f);
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
      *piVar1 = *piVar1 + 1;
      if ((*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) == 6) ||
         (7 < *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f))) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 0;
      }
      iVar4 = FUN_26f4_1d73(param_1,iVar4 + iVar2,uVar3);
      if (iVar4 == 0) {
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
             -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
        FUN_1581_0178(1,0xf);
      }
      else {
        iVar4 = FUN_26f4_2227(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                              uVar3,1);
        if (iVar4 == 0) {
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 6;
        }
      }
      param_2 = 1;
    }
  }
  return param_2;
}


