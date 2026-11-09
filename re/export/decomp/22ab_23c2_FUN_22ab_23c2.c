// FUN_22ab_23c2 @ 22ab:23c2

void __cdecl16far FUN_22ab_23c2(void)

{
  int iVar1;
  int iVar2;
  undefined1 local_5e [16];
  undefined1 local_4e [32];
  undefined1 local_2e [32];
  undefined1 local_e [12];
  
  iVar1 = FUN_22ab_1f69((char *)s_SAVE_GAME_2cff_1460,0x2cff,0x146a,0x2cff);
  if (-1 < iVar1) {
    FUN_2b9f_0008(local_e);
    FUN_1353_0861(DAT_2cff_b6ae,DAT_2cff_b6b0,0x14,iVar1 * 8 + 0xd,2,local_e);
    if ((DAT_2cff_3c86 != 0x1b) && (iVar2 = FUN_2b9a_000b(local_e), iVar2 != 0)) {
      FUN_2b9f_0008((undefined1 *)&DAT_2cff_bab4 + iVar1 * 0xc,0x2cff,local_e);
      FUN_2b85_0085(iVar1,local_5e);
      FUN_2b9f_0008(local_4e);
      FUN_2b96_0009(local_4e);
      FUN_2b96_0009(local_4e);
      FUN_2b9f_0008(local_2e);
      FUN_2b96_0009(local_2e);
      FUN_2b96_0009(local_2e);
      FUN_22ab_0c9a(local_4e);
      iVar1 = FUN_2b4b_000c(0x1470,0x2cff,0);
      if (iVar1 != -1) {
        FUN_2b56_0007(iVar1);
        FUN_105f_0004(0x1475,0x2cff,local_2e);
      }
      FUN_22ab_0a39();
    }
  }
  return;
}


