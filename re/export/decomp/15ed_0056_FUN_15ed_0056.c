// FUN_15ed_0056 @ 15ed:0056

undefined2 __cdecl16near FUN_15ed_0056(void)

{
  char cVar1;
  char cVar2;
  undefined2 uVar3;
  
  uVar3 = CONCAT11((char)((uint)DAT_2cff_0bf6 >> 8),(char)DAT_2cff_0bf6 + '\x06');
  out(uVar3,1);
  cVar1 = in(uVar3);
  do {
    cVar1 = cVar1 + '\x01';
  } while (cVar1 != '\0');
  out(uVar3,0);
  cVar1 = ' ';
  do {
    cVar2 = FUN_15ed_0016();
    if (cVar2 == -0x56) {
      return 0;
    }
    cVar1 = cVar1 + -1;
  } while (cVar1 != '\0');
  return 2;
}


