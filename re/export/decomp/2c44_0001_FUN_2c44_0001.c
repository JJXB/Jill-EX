// FUN_2c44_0001 @ 2c44:0001

undefined2 __cdecl16far FUN_2c44_0001(void)

{
  code *pcVar1;
  undefined2 uVar2;
  undefined2 uVar3;
  bool bVar4;
  undefined4 uVar5;
  
  bVar4 = false;
  pcVar1 = (code *)swi(0x21);
  uVar5 = (*pcVar1)();
  uVar3 = (int)uVar5;
  if (!bVar4) {
    bVar4 = false;
    pcVar1 = (code *)swi(0x21);
    uVar2 = (*pcVar1)((int)((ulong)uVar5 >> 0x10));
    uVar3 = uVar2;
    if (!bVar4) {
      pcVar1 = (code *)swi(0x21);
      uVar3 = (*pcVar1)();
      if (!bVar4) {
        return uVar2;
      }
    }
  }
  uVar3 = FUN_2adf_000c(uVar3);
  return uVar3;
}


