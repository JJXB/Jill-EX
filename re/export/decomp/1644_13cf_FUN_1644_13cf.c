// FUN_1644_13cf @ 1644:13cf

int __cdecl16far FUN_1644_13cf(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  
  if (param_2 == 0) {
    iVar2 = DAT_2cff_bb46 * 0x100;
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) {
      iVar2 = iVar2 + (uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f)) * 10
                      + *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
    }
    else if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 0) {
      iVar2 = iVar2 + *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) + 4;
    }
    else {
      iVar2 = iVar2 + (10 - *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f));
    }
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,iVar2,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      param_2 = FUN_26f4_25b8(param_1);
    }
  }
  else if (param_2 == 2) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
      *piVar1 = *piVar1 + 1;
      if (4 < *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 0;
      }
      iVar2 = FUN_26f4_1f35(param_1,*(undefined2 *)
                                     ((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),0);
      if (iVar2 == 0) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 5;
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
             -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
      }
    }
    else {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f);
      *piVar1 = *piVar1 + -1;
    }
    param_2 = 1;
  }
  return param_2;
}


