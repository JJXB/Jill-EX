// FUN_2c7e_0008 @ 2c7e:0008

undefined2 __cdecl16far FUN_2c7e_0008(void)

{
  code *pcVar1;
  undefined2 in_BX;
  
  pcVar1 = (code *)swi(0x21);
  (*pcVar1)();
  return in_BX;
}


