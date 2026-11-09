// FUN_2b65_0002 @ 2b65:0002

void __cdecl16far FUN_2b65_0002(void)

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


