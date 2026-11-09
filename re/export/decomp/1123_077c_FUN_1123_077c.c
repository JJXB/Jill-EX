// FUN_1123_077c @ 1123:077c

void __cdecl16far FUN_1123_077c(void)

{
  byte bVar1;
  
  do {
    bVar1 = in(0x3da);
  } while ((bVar1 & 8) != 0);
  do {
    bVar1 = in(0x3da);
  } while ((bVar1 & 8) == 0);
  return;
}


