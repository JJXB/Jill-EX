// FUN_15ed_01d6 @ 15ed:01d6

undefined2 __cdecl16near FUN_15ed_01d6(void)

{
  byte in_AL;
  byte bVar1;
  undefined2 *puVar2;
  byte bVar3;
  undefined2 *in_BX;
  
  puVar2 = (undefined2 *)CONCAT11((char)(in_AL + 8) >> 7,(in_AL + 8) * '\x04');
  *puVar2 = *in_BX;
  puVar2[1] = in_BX[1];
  bVar3 = '\x01' << (in_AL & 0x1f);
  bVar1 = in(0x21);
  bVar1 = bVar1 | bVar3;
  out(0x21,bVar1);
  return CONCAT11(bVar3,bVar1);
}


