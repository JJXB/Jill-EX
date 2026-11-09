// FUN_1644_3371 @ 1644:3371

int __cdecl16far FUN_1644_3371(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  
  if (param_2 == 0) {
    if (DAT_2cff_3a4d == '\0') {
      FUN_1353_0b64(DAT_2cff_b7f2,DAT_2cff_b7f4,
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),0);
    }
    else {
      FUN_1353_0b64(DAT_2cff_b7f2,DAT_2cff_b7f4,
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
    }
    param_2 = FUN_1353_074e(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 1,2,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c3 + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c5 + param_1 * 0x1f));
  }
  else if (param_2 == 2) {
    if (0 < *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)) {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
      iVar2 = *piVar1;
      *piVar1 = *piVar1 + -1;
      if (*piVar1 == 0 || SBORROW2(iVar2,1) != *piVar1 < 0) {
        FUN_26f4_1863(param_1);
        return 1;
      }
    }
    param_2 = 0;
  }
  return param_2;
}


