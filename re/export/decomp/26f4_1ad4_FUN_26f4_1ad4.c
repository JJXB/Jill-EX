// FUN_26f4_1ad4 @ 26f4:1ad4

void __cdecl16far FUN_26f4_1ad4(int param_1,int param_2,int param_3)

{
  if (param_3 < 0) {
    param_3 = 0;
  }
  else if (0x3f0 - *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f) < param_3) {
    param_3 = 0x3f0 - *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f);
  }
  if (param_2 < 0) {
    param_2 = 0;
  }
  else if (0x7f0 - *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) < param_2) {
    param_2 = 0x7f0 - *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f);
  }
  *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) = param_2;
  *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) = param_3;
  return;
}


