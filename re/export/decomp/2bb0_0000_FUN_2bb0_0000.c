// FUN_2bb0_0000 @ 2bb0:0000

undefined2 __cdecl16far FUN_2bb0_0000(void)

{
  code *pcVar1;
  undefined1 in_CF;
  undefined2 in_stack_0000000a;
  
  pcVar1 = (code *)swi(0x21);
  (*pcVar1)();
  if ((bool)in_CF) {
    in_stack_0000000a = FUN_2adf_000c();
  }
  return in_stack_0000000a;
}


