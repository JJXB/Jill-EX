// FUN_1123_0791 @ 1123:0791

/* WARNING: Removing unreachable block (ram,0x000119ea) */

void __cdecl16far FUN_1123_0791(void)

{
  uint local_4;
  
  if (DAT_2cff_3a4d == '\x04') {
    FUN_1123_03c3();
    for (local_4 = 0; local_4 < 0x100; local_4 = local_4 + 1) {
      out(DAT_2cff_0494,(undefined1)local_4);
      out(DAT_2cff_0498,*(undefined1 *)(local_4 * 3 + 0x194));
      out(DAT_2cff_0498,*(undefined1 *)(local_4 * 3 + 0x195));
      out(DAT_2cff_0498,*(undefined1 *)(local_4 * 3 + 0x196));
    }
  }
  return;
}


