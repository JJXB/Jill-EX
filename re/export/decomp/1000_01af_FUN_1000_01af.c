// FUN_1000_01af @ 1000:01af

void __stdcall16far FUN_1000_01af(void)

{
  char cVar1;
  char cVar2;
  int in_BX;
  int unaff_SI;
  
  cVar2 = '\0';
  FUN_1000_01a7();
  cVar1 = FUN_1000_010d(3);
  *(char *)(in_BX + unaff_SI) = *(char *)(in_BX + unaff_SI) + cVar1;
  *(char *)(in_BX + unaff_SI) = *(char *)(in_BX + unaff_SI) + cVar1;
  *(char *)(in_BX + unaff_SI) = *(char *)(in_BX + unaff_SI) + cVar1;
  *(char *)(in_BX + unaff_SI) = *(char *)(in_BX + unaff_SI) + cVar1;
  *(char *)(in_BX + unaff_SI) = *(char *)(in_BX + unaff_SI) + cVar1;
  cRam0000068f = cRam0000068f + cVar2;
  return;
}


