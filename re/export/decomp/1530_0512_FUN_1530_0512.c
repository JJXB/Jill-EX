// FUN_1530_0512 @ 1530:0512

byte __cdecl16far FUN_1530_0512(void)

{
  byte bVar1;
  
  bVar1 = in(0x61);
  out(0x61,bVar1 & 0xfc);
  return bVar1 & 0xfc;
}


