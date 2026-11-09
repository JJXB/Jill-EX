// FUN_2c09_00b9 @ 2c09:00b9

void __cdecl16far FUN_2c09_00b9(uint *param_1,uint *param_2)

{
  uint *puVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  undefined2 uVar6;
  uint *puVar7;
  undefined2 uVar8;
  undefined2 uVar9;
  bool bVar10;
  
  uVar8 = (undefined2)((ulong)param_2 >> 0x10);
  uVar3 = ((uint *)param_2)[1];
  uVar4 = *param_2;
  uVar9 = (undefined2)((ulong)param_1 >> 0x10);
  puVar7 = (uint *)param_1;
  uVar2 = *param_1;
  *param_1 = *param_1 + uVar4;
  puVar1 = puVar7 + 1;
  *puVar1 = *puVar1 + uVar3 + (uint)CARRY2(uVar2,uVar4);
  bVar10 = *puVar1 == 0;
  FUN_1000_036f();
  if (bVar10) {
    DAT_2cff_2848 = puVar7;
    DAT_2cff_284a = uVar9;
  }
  else {
    uVar6 = uVar8;
    iVar5 = FUN_1000_0314();
    *(undefined2 *)(iVar5 + 6) = uVar9;
    *(undefined2 *)(iVar5 + 4) = puVar7;
  }
  FUN_2ae9_001f((uint *)param_2,uVar8);
  return;
}


