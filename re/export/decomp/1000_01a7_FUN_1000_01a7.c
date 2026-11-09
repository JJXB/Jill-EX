// FUN_1000_01a7 @ 1000:01a7

void __cdecl16near FUN_1000_01a7(void)

{
  code *pcVar1;
  
  pcVar1 = (code *)swi(0x21);
  (*pcVar1)();
  return;
}


