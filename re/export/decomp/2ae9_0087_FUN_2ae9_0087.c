// FUN_2ae9_0087 @ 2ae9:0087

int * __cdecl16far FUN_2ae9_0087(uint *param_1,uint param_2,int param_3)

{
  uint *puVar1;
  uint uVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  bool bVar7;
  undefined4 local_6;
  
  iVar6 = (int)((ulong)param_1 >> 0x10);
  uVar2 = *param_1;
  *param_1 = *param_1 - param_2;
  puVar1 = (uint *)param_1 + 1;
  *puVar1 = (*puVar1 - param_3) - (uint)(uVar2 < param_2);
  iVar5 = iVar6;
  piVar3 = (int *)FUN_1000_0314();
  local_6 = (int *)CONCAT22(iVar5,piVar3);
  param_3 = param_3 + (uint)(0xfffe < param_2);
  bVar7 = param_3 == 0;
  piVar3[1] = param_3;
  *local_6 = param_2 + 1;
  piVar3[3] = iVar6;
  piVar3[2] = (int)(uint *)param_1;
  FUN_1000_036f();
  if (bVar7) {
    DAT_2cff_2848 = piVar3;
    DAT_2cff_284a = iVar5;
  }
  else {
    iVar6 = iVar5;
    iVar4 = FUN_1000_0314();
    *(int *)(iVar4 + 6) = iVar5;
    *(int *)(iVar4 + 4) = (int)piVar3;
  }
  return piVar3 + 4;
}


