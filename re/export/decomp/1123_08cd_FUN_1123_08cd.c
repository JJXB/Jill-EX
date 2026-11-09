// FUN_1123_08cd @ 1123:08cd

/* WARNING: Removing unreachable block (ram,0x00011b25) */
/* WARNING: Removing unreachable block (ram,0x00011b19) */
/* WARNING: Removing unreachable block (ram,0x00011b39) */
/* WARNING: Removing unreachable block (ram,0x00011b45) */

void __cdecl16far FUN_1123_08cd(void)

{
  uint local_4;
  
  if (DAT_2cff_3a4d == '\x04') {
    for (local_4 = 0; local_4 < 0x100; local_4 = local_4 + 1) {
      out(DAT_2cff_0494,(undefined1)local_4);
      out(DAT_2cff_0498,0);
      out(DAT_2cff_0498,0);
      out(DAT_2cff_0498,0);
    }
  }
  return;
}


