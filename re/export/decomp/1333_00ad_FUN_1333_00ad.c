// FUN_1333_00ad @ 1333:00ad

void __cdecl16far FUN_1333_00ad(void)

{
  uint local_16 [10];
  
  local_16[0] = 0x200;
  FUN_2c8f_000b(0x16,local_16);
  DAT_2cff_3c67 = (byte)local_16[0] & 1;
  DAT_2cff_3c68 = (byte)(local_16[0] >> 1) & 1;
  DAT_2cff_3a62 = (byte)(local_16[0] >> 2) & 1;
  DAT_2cff_3a63 = (byte)(local_16[0] >> 3) & 1;
  DAT_2cff_3c66 = (byte)(local_16[0] >> 5) & 1;
  DAT_2cff_3c65 = (byte)local_16[0] & 1 | DAT_2cff_3c68;
  return;
}


