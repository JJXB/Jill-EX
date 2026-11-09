// FUN_1000_010d @ 1000:010d

void __stdcall16far FUN_1000_010d(void)

{
  code *pcVar1;
  char cVar2;
  char cVar3;
  int in_BX;
  int unaff_SI;
  
  FUN_1000_0172();
  (*pcRam00002a48)(0x1000);
  pcVar1 = (code *)swi(0x21);
  (*pcVar1)();
  cVar3 = '\0';
  FUN_1000_01a7();
  cVar2 = FUN_1000_010d();
  *(char *)(in_BX + unaff_SI) = *(char *)(in_BX + unaff_SI) + cVar2;
  *(char *)(in_BX + unaff_SI) = *(char *)(in_BX + unaff_SI) + cVar2;
  *(char *)(in_BX + unaff_SI) = *(char *)(in_BX + unaff_SI) + cVar2;
  *(char *)(in_BX + unaff_SI) = *(char *)(in_BX + unaff_SI) + cVar2;
  *(char *)(in_BX + unaff_SI) = *(char *)(in_BX + unaff_SI) + cVar2;
  cRam0000068f = cRam0000068f + cVar3;
  return;
}


