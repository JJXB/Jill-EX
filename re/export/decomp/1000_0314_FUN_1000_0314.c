// FUN_1000_0314 @ 1000:0314

byte __cdecl16far FUN_1000_0314(void)

{
  char in_AL;
  int in_CX;
  byte in_BL;
  
  if (in_CX < 0) {
    return in_AL - (~in_BL + 1) & 0xf;
  }
  return in_AL + in_BL & 0xf;
}


