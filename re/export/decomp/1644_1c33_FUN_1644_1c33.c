// FUN_1644_1c33 @ 1644:1c33

int __cdecl16far FUN_1644_1c33(int param_1,int param_2,int param_3)

{
  uint *puVar1;
  int iVar2;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bb50 * 0x100 +
                            (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 0)
                            * 3 + *(int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      param_2 = FUN_26f4_25b8(param_1);
    }
  }
  else if (param_2 == 2) {
    *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) =
         *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + 1U & 3;
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) == 0) {
      puVar1 = (uint *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f);
      *puVar1 = *puVar1 ^ 1;
      iVar2 = FUN_26f4_1f35(param_1,*(undefined2 *)
                                     ((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),0);
      if (iVar2 == 0) {
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
             -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
      }
      param_2 = 1;
    }
    else {
      param_2 = 0;
    }
  }
  return param_2;
}


