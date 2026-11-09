// FUN_1000_0414 @ 1000:0414

void __cdecl16far FUN_1000_0414(void)

{
  code *pcVar1;
  undefined2 unaff_BP;
  
  pcVar1 = (code *)swi(0x10);
  DAT_2cff_bf52 = unaff_BP;
  (*pcVar1)();
  return;
}


