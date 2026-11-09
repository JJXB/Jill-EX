// FUN_15ed_02c9 @ 15ed:02c9

undefined1 __cdecl16near FUN_15ed_02c9(void)

{
  byte bVar1;
  undefined1 uVar2;
  
  out(10,5);
  bVar1 = in(0x21);
  out(0x21,bVar1 | '\x01' << (DAT_2cff_0bf8 & 0x1f));
  FUN_15ed_01d6();
  DAT_2cff_0bf9 = 0;
  uVar2 = in(CONCAT11((char)((uint)DAT_2cff_0bf6 >> 8),(char)DAT_2cff_0bf6 + '\x0e'));
  return uVar2;
}


