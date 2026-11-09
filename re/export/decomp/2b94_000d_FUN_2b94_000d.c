// FUN_2b94_000d @ 2b94:000d

undefined2 __cdecl16far FUN_2b94_000d(void)

{
  code *pcVar1;
  undefined2 uVar2;
  undefined1 in_CF;
  
  pcVar1 = (code *)swi(0x21);
  (*pcVar1)();
  if ((bool)in_CF) {
    uVar2 = FUN_2adf_000c();
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}


