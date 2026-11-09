// FUN_2b31_000c @ 2b31:000c

void FUN_2b31_000c(void)

{
  code *pcVar1;
  undefined1 in_CF;
  
  pcVar1 = (code *)swi(0x21);
  (*pcVar1)();
  if ((bool)in_CF) {
    FUN_2adf_000c();
  }
  return;
}


