// FUN_1000_0272 @ 1000:0272

char __cdecl16near FUN_1000_0272(void)

{
  char cVar1;
  char cVar2;
  char *unaff_SI;
  
  cVar1 = *unaff_SI;
  cVar2 = '\0';
  if (((cVar1 != '\"') && (cVar2 = cVar1, cVar1 == '\\')) && (unaff_SI[1] == '\"')) {
    cVar2 = unaff_SI[1];
  }
  return cVar2;
}


