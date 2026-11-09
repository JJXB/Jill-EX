// FUN_1a41_0a3b @ 1a41:0a3b

int __cdecl16far FUN_1a41_0a3b(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bb86 * 0x100 +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) + 0xc +
                            (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) / 8) * -3,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 2) {
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
    *piVar1 = *piVar1 + 1;
    if ((*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) < 0x28) &&
       (iVar2 = FUN_26f4_1da8(param_1), iVar2 != 0)) {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      *piVar1 = *piVar1 + 1;
      if (0xc < *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0xc;
      }
      FUN_26f4_1ad4(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                    *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                    *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
    }
    else {
      FUN_26f4_1863(param_1);
    }
    param_2 = 1;
  }
  return param_2;
}


