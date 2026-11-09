// FUN_15ed_02f3 @ 15ed:02f3

void __cdecl16near FUN_15ed_02f3(void)

{
  byte in_AL;
  byte bVar1;
  int iVar2;
  
  iVar2 = 0x40;
  do {
    bVar1 = in(CONCAT11((char)((uint)DAT_2cff_0bf6 >> 8),(char)DAT_2cff_0bf6 + '\b'));
    if ((in_AL & 0xe0) == (bVar1 & 0xe0)) {
      return;
    }
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  return;
}


