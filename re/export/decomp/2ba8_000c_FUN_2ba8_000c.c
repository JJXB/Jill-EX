// FUN_2ba8_000c @ 2ba8:000c

void __cdecl16far FUN_2ba8_000c(undefined1 *param_1,undefined1 *param_2,uint param_3)

{
  undefined1 *puVar1;
  undefined2 *puVar2;
  undefined1 *puVar3;
  undefined2 *puVar4;
  uint uVar5;
  uint uVar6;
  undefined1 *puVar7;
  undefined2 *puVar8;
  undefined1 *puVar9;
  undefined2 *puVar10;
  undefined2 uVar11;
  undefined2 uVar12;
  byte in_CF;
  
  FUN_1000_036f(0x2cff);
  uVar5 = (uint)in_CF;
  uVar12 = (undefined2)((ulong)param_1 >> 0x10);
  puVar7 = (undefined1 *)param_1;
  uVar11 = (undefined2)((ulong)param_2 >> 0x10);
  puVar9 = (undefined1 *)param_2;
  if (uVar5 != 0) {
    puVar7 = puVar7 + (param_3 - 1);
    puVar9 = puVar9 + (param_3 - 1);
  }
  if (((uint)puVar9 & 1) != 0) {
    if (param_3 == 0) {
      return;
    }
    puVar3 = puVar9;
    puVar9 = puVar9 + (uint)in_CF * -2 + 1;
    puVar1 = puVar7;
    puVar7 = puVar7 + (uint)in_CF * -2 + 1;
    *puVar3 = *puVar1;
    param_3 = param_3 - 1;
  }
  puVar8 = (undefined2 *)(puVar7 + -uVar5);
  puVar10 = (undefined2 *)(puVar9 + -uVar5);
  for (uVar6 = param_3 >> 1; uVar6 != 0; uVar6 = uVar6 - 1) {
    puVar4 = puVar10;
    puVar10 = puVar10 + (uint)in_CF * -2 + 1;
    puVar2 = puVar8;
    puVar8 = puVar8 + (uint)in_CF * -2 + 1;
    *puVar4 = *puVar2;
  }
  if ((param_3 & 1) != 0) {
    *(undefined1 *)((int)puVar10 + uVar5) = *(undefined1 *)((int)puVar8 + uVar5);
  }
  return;
}


