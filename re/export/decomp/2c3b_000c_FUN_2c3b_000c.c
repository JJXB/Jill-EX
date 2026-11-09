// FUN_2c3b_000c @ 2c3b:000c

int FUN_2c3b_000c(undefined2 param_1)

{
  code *pcVar1;
  int iVar2;
  undefined2 unaff_BP;
  undefined1 in_CF;
  
  pcVar1 = (code *)swi(0x21);
  iVar2 = (*pcVar1)();
  if ((bool)in_CF) {
    iVar2 = FUN_2adf_000c(iVar2);
  }
  else {
    *(undefined2 *)(iVar2 * 2 + 0x2954) = param_1;
  }
  return iVar2;
}


