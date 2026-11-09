// FUN_1415_0004 @ 1415:0004

bool __cdecl16far FUN_1415_0004(void)

{
  byte bVar1;
  
  bVar1 = in(0x201);
  return (bVar1 & 0x10) == 0;
}


