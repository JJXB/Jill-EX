// FUN_1a41_4c8f @ 1a41:4c8f

void __cdecl16far FUN_1a41_4c8f(int param_1,int param_2)

{
  int *piVar1;
  
  if (param_2 != 0) {
    if (param_2 == 1) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) {
        FUN_22ab_05b4(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c3 + param_1 * 0x1f),
                      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c5 + param_1 * 0x1f),7);
        FUN_1581_0178(3,0x14);
      }
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 8;
    }
    else if ((param_2 == 2) && (0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f))) {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f);
      *piVar1 = *piVar1 + -1;
    }
  }
  return;
}


