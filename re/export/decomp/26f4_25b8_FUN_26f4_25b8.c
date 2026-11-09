// FUN_26f4_25b8 @ 26f4:25b8

void __cdecl16far FUN_26f4_25b8(int param_1)

{
  if ((*(int *)((int)(undefined2 *)&DAT_2cff_96c9 + param_1 * 0x1f) == 0) &&
     ((((undefined2 *)&DAT_2cff_be2c)[DAT_2cff_96b9] & 2) == 0)) {
    FUN_26f4_05e7(1,0);
  }
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c9 + param_1 * 0x1f) = 3;
  return;
}


