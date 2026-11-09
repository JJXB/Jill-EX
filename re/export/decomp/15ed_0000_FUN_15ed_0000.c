// FUN_15ed_0000 @ 15ed:0000

void __cdecl16near FUN_15ed_0000(void)

{
  undefined1 in_AL;
  char cVar1;
  int iVar2;
  undefined2 in_DX;
  
  iVar2 = 0x200;
  do {
    cVar1 = in(in_DX);
    if (-1 < cVar1) {
      out(in_DX,in_AL);
      return;
    }
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  return;
}


