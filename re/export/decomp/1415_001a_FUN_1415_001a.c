// FUN_1415_001a @ 1415:001a

bool __cdecl16far FUN_1415_001a(void)

{
  byte bVar1;
  
  bVar1 = in(0x201);
  return (bVar1 & 0x20) == 0;
}


