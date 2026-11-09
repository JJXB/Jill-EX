// FUN_15ed_0041 @ 15ed:0041

undefined1 __cdecl16near FUN_15ed_0041(void)

{
  char cVar1;
  undefined1 uVar2;
  
  uVar2 = (undefined1)((uint)DAT_2cff_0bf6 >> 8);
  do {
    cVar1 = in(CONCAT11(uVar2,(char)DAT_2cff_0bf6 + '\x0e'));
  } while (-1 < cVar1);
  uVar2 = in(CONCAT11(uVar2,(char)DAT_2cff_0bf6 + '\n'));
  return uVar2;
}


