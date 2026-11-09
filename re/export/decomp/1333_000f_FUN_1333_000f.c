// FUN_1333_000f @ 1333:000f

int __cdecl16far FUN_1333_000f(void)

{
  int iVar1;
  uint local_16 [9];
  uint local_4;
  
  local_16[0] = 0x100;
  FUN_2c8f_000b(0x16,local_16);
  if ((local_4 >> 6 & 1) == 0) {
    iVar1 = (local_16[0] >> 8 | 0x80) * (uint)((local_16[0] & 0xff) == 0) + (local_16[0] & 0xff);
  }
  else {
    iVar1 = 0;
  }
  return iVar1;
}


