// FUN_2c09_0216 @ 2c09:0216

void __cdecl16far FUN_2c09_0216(uint *param_1)

{
  uint *puVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint *puVar5;
  uint *puVar6;
  undefined2 uVar7;
  uint *puVar8;
  undefined2 uVar9;
  bool bVar10;
  undefined4 local_a;
  uint uStack_4;
  
  uVar9 = (undefined2)((ulong)param_1 >> 0x10);
  puVar8 = (uint *)param_1;
  uVar2 = *param_1;
  *param_1 = *param_1 - 1;
  puVar8[1] = puVar8[1] - (uint)(uVar2 == 0);
  uVar7 = uVar9;
  puVar6 = (uint *)FUN_1000_0314();
  local_a = (uint *)CONCAT22(uVar7,puVar6);
  puVar5 = *(uint **)(puVar8 + 2);
  uStack_4 = (uint)((ulong)puVar5 >> 0x10);
  bVar10 = (*puVar5 & 1) == 0;
  if (bVar10) {
    FUN_1000_036f();
    if (!bVar10) {
      uVar3 = puVar8[1];
      uVar4 = *param_1;
      uVar2 = *puVar5;
      *puVar5 = *puVar5 + uVar4;
      puVar1 = (uint *)puVar5 + 1;
      *puVar1 = *puVar1 + uVar3 + (uint)CARRY2(uVar2,uVar4);
      puVar6[3] = uStack_4;
      puVar6[2] = (uint)(uint *)puVar5;
      param_1 = puVar5;
      goto LAB_2c09_02b6;
    }
  }
  FUN_2c09_001a(puVar8,uVar9);
LAB_2c09_02b6:
  if ((*local_a & 1) == 0) {
    FUN_2c09_00b9((uint *)param_1,param_1._2_2_,puVar6,uVar7);
  }
  return;
}


