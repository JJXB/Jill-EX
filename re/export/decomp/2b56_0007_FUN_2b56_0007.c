// FUN_2b56_0007 @ 2b56:0007

undefined2 __cdecl16far FUN_2b56_0007(int param_1)

{
  code *pcVar1;
  undefined2 uVar2;
  undefined1 in_CF;
  
  pcVar1 = (code *)swi(0x21);
  uVar2 = (*pcVar1)();
  if ((bool)in_CF) {
    uVar2 = FUN_2adf_000c(uVar2);
  }
  else {
    *(undefined2 *)(param_1 * 2 + 0x2954) = 0xffff;
    uVar2 = 0;
  }
  return uVar2;
}


