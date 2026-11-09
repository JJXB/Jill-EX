// FUN_1069_004d @ 1069:004d

void __cdecl16far FUN_1069_004d(void)

{
  int local_4;
  
  if ((DAT_2cff_3a4d & 0xfe) == 0) {
    for (local_4 = 0; local_4 < 0x100; local_4 = local_4 + 1) {
      *(byte *)(local_4 + 0x2e5c) = (byte)local_4 & 3;
    }
  }
  else if ((DAT_2cff_3a4d & 0xfe) == 2) {
    FUN_2ba2_0001(0x2e5c,0x2cff,0x94,0x2cff,0x100);
  }
  else if ((DAT_2cff_3a4d & 0xfe) == 4) {
    for (local_4 = 0; local_4 < 0x100; local_4 = local_4 + 1) {
      *(undefined1 *)(local_4 + 0x2e5c) = (byte)local_4;
    }
  }
  for (local_4 = 0; local_4 < 0x100; local_4 = local_4 + 1) {
    *(undefined1 *)(local_4 + 0x2e5c) = (byte)local_4;
  }
  return;
}


