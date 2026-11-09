// FUN_1123_03c3 @ 1123:03c3

void __cdecl16far FUN_1123_03c3(void)

{
  byte bVar1;
  
  do {
    bVar1 = in(0x3da);
  } while ((bVar1 & 8) == 0);
  return;
}


