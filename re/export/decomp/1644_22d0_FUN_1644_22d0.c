// FUN_1644_22d0 @ 1644:22d0

int __cdecl16far FUN_1644_22d0(int param_1,int param_2)

{
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bb58 * 0x100 +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    param_2 = 0;
  }
  else if (param_2 == 2) {
    param_2 = 0;
  }
  return param_2;
}


