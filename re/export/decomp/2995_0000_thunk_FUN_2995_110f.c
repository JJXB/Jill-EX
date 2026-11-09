// thunk_FUN_2995_110f @ 2995:0000

undefined2 __cdecl16far thunk_FUN_2995_110f(void)

{
  undefined2 uVar1;
  uint in_BX;
  
  if (DAT_2995_01e4 != '\0') {
    return 0xfff8;
  }
  DAT_2995_01e4 = 1;
  if ((int)in_BX < 0) {
    if (2 < ~in_BX) {
      DAT_2995_01e4 = 0;
      return 0xffff;
    }
    uVar1 = (*(code *)*(undefined2 *)(~in_BX * 2 + 0x27f))();
  }
  else {
    if (0xe < in_BX) {
      DAT_2995_01e4 = 0;
      return 0xffff;
    }
    uVar1 = (*(code *)*(undefined2 *)(in_BX * 2 + 0x261))();
  }
  DAT_2995_01e4 = 0;
  return uVar1;
}


