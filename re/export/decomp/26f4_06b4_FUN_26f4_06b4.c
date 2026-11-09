// FUN_26f4_06b4 @ 26f4:06b4

void __cdecl16far FUN_26f4_06b4(int param_1,int *param_2,int *param_3)

{
  *param_2 = (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) < DAT_2cff_96ad) -
             (uint)(DAT_2cff_96ad < *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f));
  *param_3 = (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) < DAT_2cff_96af) -
             (uint)(DAT_2cff_96af < *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  return;
}


