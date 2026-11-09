// FUN_1581_03ef @ 1581:03ef

void __cdecl16far FUN_1581_03ef(int param_1)

{
  int iVar1;
  int in_DX;
  
  DAT_2cff_3f0c = 0;
  DAT_2cff_3f0a = 0;
  if (*(int *)(param_1 * 2 + 0x41da) != 0) {
    DAT_2cff_439a = *(undefined2 *)(param_1 * 2 + 0x41da);
    DAT_2cff_3f0a = FUN_2ae9_000a(DAT_2cff_439a);
    DAT_2cff_3f0c = in_DX;
    if (DAT_2cff_3f0a != 0 || in_DX != 0) {
      FUN_2b82_0004(DAT_2cff_439c,*(undefined2 *)(param_1 * 4 + 0x42f6),
                    *(undefined2 *)(param_1 * 4 + 0x42f8),0);
      iVar1 = FUN_2b58_000d(DAT_2cff_439c,DAT_2cff_3f0a,DAT_2cff_3f0c,
                            *(undefined2 *)(param_1 * 2 + 0x41da));
      if (iVar1 == -1) {
        DAT_2cff_3f0c = 0;
        DAT_2cff_3f0a = 0;
      }
    }
  }
  return;
}


