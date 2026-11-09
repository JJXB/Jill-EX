// FUN_26f4_1b8f @ 26f4:1b8f

undefined2 __cdecl16far FUN_26f4_1b8f(int param_1,int param_2,int param_3)

{
  undefined2 uVar1;
  int local_4;
  
  param_2 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) + param_2;
  param_3 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + param_3;
  if ((*(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f) + param_3 & 0xfU) == 0) {
    for (local_4 = param_2 >> 4;
        local_4 < *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) + param_2 + 0xf >> 4;
        local_4 = local_4 + 1) {
      if ((*(uint *)(((undefined2 *)&DAT_2cff_43a6)
                     [local_4 * 0x40 +
                      (*(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f) + param_3 + -1)
                      / 0x10 + 1] * 8 + -0x7c12) & 3) == 3) {
        return 0;
      }
    }
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}


