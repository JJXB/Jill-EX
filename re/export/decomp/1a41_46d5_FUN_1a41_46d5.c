// FUN_1a41_46d5 @ 1a41:46d5

int __cdecl16far FUN_1a41_46d5(int param_1,int param_2)

{
  int iVar1;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,DAT_2cff_bb8e * 0x100 + 0x22,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 2) {
    iVar1 = FUN_2ca6_0015();
    if (iVar1 % 0xc == 0) {
      FUN_26f4_1892(0x3a,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) + 2,
                    *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 4);
      param_2 = 1;
    }
    else {
      param_2 = 0;
    }
  }
  return param_2;
}


