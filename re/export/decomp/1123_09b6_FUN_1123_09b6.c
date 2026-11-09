// FUN_1123_09b6 @ 1123:09b6

void __cdecl16far FUN_1123_09b6(void)

{
  int iVar1;
  int iVar2;
  undefined2 unaff_SS;
  undefined1 local_304 [768];
  int local_4;
  
  if (DAT_2cff_3a4d == '\x04') {
    for (iVar2 = 0x3f; -1 < iVar2; iVar2 = iVar2 + -2) {
      for (iVar1 = 0; iVar1 < 0x300; iVar1 = iVar1 + 1) {
        local_4 = (int)((uint)*(byte *)(iVar1 + 0x194) * iVar2) >> 6;
        local_304[iVar1] = (undefined1)local_4;
      }
      FUN_1123_03c3();
      out(DAT_2cff_0494,0);
      for (iVar1 = 0; iVar1 < 0x300; iVar1 = iVar1 + 1) {
        out(DAT_2cff_0498,local_304[iVar1]);
      }
    }
  }
  return;
}


