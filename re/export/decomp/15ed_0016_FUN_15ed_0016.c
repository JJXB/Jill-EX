// FUN_15ed_0016 @ 15ed:0016

char __cdecl16near FUN_15ed_0016(void)

{
  char cVar1;
  int iVar2;
  undefined1 uVar3;
  
  uVar3 = (undefined1)((uint)DAT_2cff_0bf6 >> 8);
  iVar2 = 0x200;
  do {
    cVar1 = in(CONCAT11(uVar3,(char)DAT_2cff_0bf6 + '\x0e'));
    if (cVar1 < '\0') {
      cVar1 = in(CONCAT11(uVar3,(char)DAT_2cff_0bf6 + '\n'));
      return cVar1;
    }
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  return cVar1;
}


