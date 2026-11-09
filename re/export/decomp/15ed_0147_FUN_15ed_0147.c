// FUN_15ed_0147 @ 15ed:0147

void __cdecl16near FUN_15ed_0147(void)

{
  char cVar1;
  undefined2 uVar2;
  
  uVar2 = CONCAT11((char)((uint)DAT_2cff_0bf6 >> 8),(char)DAT_2cff_0bf6 + '\f');
  do {
    if (DAT_2cff_0bf9 == 0) {
      return;
    }
    cVar1 = in(uVar2);
  } while (-1 < cVar1);
  do {
    cVar1 = in(uVar2);
  } while (cVar1 < '\0');
  out(uVar2,0xd0);
  return;
}


