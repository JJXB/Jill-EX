// FUN_1530_04e6 @ 1530:04e6

undefined2 __cdecl16far FUN_1530_04e6(uint param_1)

{
  byte bVar1;
  undefined2 uVar2;
  undefined1 uVar3;
  
  uVar2 = 0x34dd;
  if (0x12 < param_1) {
    uVar3 = (undefined1)(0x1234dd / (ulong)param_1 >> 8);
    bVar1 = in(0x61);
    if ((bVar1 & 3) == 0) {
      out(0x61,bVar1 | 3);
      out(0x43,0xb6);
    }
    out(0x42,(char)(0x1234dd / (ulong)param_1));
    uVar2 = CONCAT11(uVar3,uVar3);
    out(0x42,uVar3);
  }
  return uVar2;
}


