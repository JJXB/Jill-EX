// FUN_1333_0066 @ 1333:0066

int __cdecl16far FUN_1333_0066(void)

{
  uint local_16 [10];
  
  local_16[0] = 0;
  FUN_2c8f_000b(0x16,local_16);
  return (local_16[0] >> 8 | 0x80) * (uint)((local_16[0] & 0xff) == 0) + (local_16[0] & 0xff);
}


