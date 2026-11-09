// FUN_2b31_002b @ 2b31:002b

void FUN_2b31_002b(void)

{
  code *pcVar1;
  
  pcVar1 = (code *)swi(0x21);
  (*pcVar1)();
  return;
}


