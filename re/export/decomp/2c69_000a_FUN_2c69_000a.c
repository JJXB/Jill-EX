// FUN_2c69_000a @ 2c69:000a

uint __cdecl16far FUN_2c69_000a(byte *param_1)

{
  ulong uVar1;
  bool bVar2;
  uint uVar3;
  uint uVar4;
  byte bVar5;
  byte *pbVar6;
  undefined2 uVar7;
  
  uVar7 = (undefined2)((ulong)param_1 >> 0x10);
  pbVar6 = (byte *)param_1;
  uVar3 = 0;
  do {
    bVar5 = *pbVar6;
    pbVar6 = pbVar6 + 1;
  } while ((*(byte *)(bVar5 + 0x2853) & 1) != 0);
  bVar2 = false;
  if (bVar5 != 0x2b) {
    if (bVar5 != 0x2d) goto LAB_2c69_003b;
    bVar2 = true;
  }
  do {
    bVar5 = *pbVar6;
    pbVar6 = pbVar6 + 1;
LAB_2c69_003b:
    if ((0x39 < bVar5) || (bVar5 < 0x30)) goto LAB_2c69_006f;
    uVar1 = (ulong)uVar3;
    uVar4 = (uint)(uVar1 * 10);
    uVar3 = uVar4 + (byte)(bVar5 - 0x30);
  } while ((char)((char)(uVar1 * 10 >> 0x10) + CARRY2(uVar4,(uint)(byte)(bVar5 - 0x30))) == '\0');
  while( true ) {
    bVar5 = *pbVar6;
    pbVar6 = pbVar6 + 1;
    if ((0x39 < bVar5) || (bVar5 < 0x30)) break;
    uVar3 = uVar3 * 10 + (uint)(byte)(bVar5 - 0x30);
  }
LAB_2c69_006f:
  if (bVar2) {
    uVar3 = -uVar3;
  }
  return uVar3;
}


