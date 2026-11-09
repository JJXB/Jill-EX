// FUN_1123_00eb @ 1123:00eb

void __cdecl16far FUN_1123_00eb(undefined4 param_1,uint param_2,int param_3,int param_4)

{
  byte bVar1;
  byte bVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  uint uVar6;
  undefined2 uVar7;
  undefined2 uVar8;
  
  if ((int)param_2 < 0) {
    param_2 = -param_2;
    uVar6 = 2;
  }
  else if ((param_2 & 0x4000) == 0) {
    uVar6 = *(uint *)(((int)param_2 >> 8) * 2 + 0x302e) & 1;
  }
  else {
    uVar6 = 3;
    param_2 = param_2 ^ 0x4000;
  }
  iVar3 = (int)param_2 >> 8;
  if (((undefined2 *)&DAT_2cff_312e)[iVar3 * 2] == 0 &&
      ((undefined2 *)&DAT_2cff_3130)[iVar3 * 2] == 0) {
    ((undefined2 *)&DAT_2cff_2fae)[iVar3] = 1;
    FUN_1069_094d();
    iVar5 = DAT_2cff_322e;
    if (((undefined2 *)&DAT_2cff_312e)[iVar3 * 2] == 0 &&
        ((undefined2 *)&DAT_2cff_3130)[iVar3 * 2] == 0) {
      ((undefined2 *)&DAT_2cff_3130)[iVar3 * 2] = DAT_2cff_3230;
      ((undefined2 *)&DAT_2cff_312e)[iVar3 * 2] = iVar5;
    }
  }
  if ((((undefined2 *)&DAT_2cff_3130)[iVar3 * 2] != DAT_2cff_3230) ||
     (((undefined2 *)&DAT_2cff_312e)[iVar3 * 2] != DAT_2cff_322e)) {
    piVar4 = (int *)((int)*(undefined4 *)((undefined2 *)&DAT_2cff_312e + iVar3 * 2) +
                    (param_2 & 0xff) * 4);
    uVar8 = (undefined2)((ulong)*(undefined4 *)((undefined2 *)&DAT_2cff_312e + iVar3 * 2) >> 0x10);
    uVar7 = (undefined2)((ulong)*(undefined4 *)((undefined2 *)&DAT_2cff_312e + iVar3 * 2) >> 0x10);
    iVar3 = (int)*(undefined4 *)((undefined2 *)&DAT_2cff_312e + iVar3 * 2) +
            *(int *)CONCAT22(uVar8,piVar4);
    bVar1 = *(byte *)(piVar4 + 1);
    bVar2 = *(byte *)((int)piVar4 + 3);
    uVar8 = (undefined2)((ulong)param_1 >> 0x10);
    iVar5 = (int)param_1;
    param_3 = param_3 - *(int *)(iVar5 + 8);
    param_4 = param_4 - *(int *)(iVar5 + 10);
    if ((param_4 < *(int *)(iVar5 + 6)) &&
       (((SCARRY2((uint)bVar2,param_4) == (int)((uint)bVar2 + param_4) < 0 &&
         (param_3 < *(int *)(iVar5 + 4))) &&
        (SCARRY2((uint)bVar1 * DAT_2cff_3a52,param_3) ==
         (int)((uint)bVar1 * DAT_2cff_3a52 + param_3) < 0)))) {
      if ((DAT_2cff_3a4d & 0xfe) == 0) {
        FUN_11f4_000e(iVar5,uVar8,param_3,param_4,bVar1,bVar2,iVar3,uVar7,uVar6);
      }
      else if ((DAT_2cff_3a4d & 0xfe) == 2) {
        FUN_11f4_027c(iVar5,uVar8,param_3,param_4,bVar1,bVar2,iVar3,uVar7,uVar6);
      }
      else if ((DAT_2cff_3a4d & 0xfe) == 4) {
        FUN_11f4_0563(iVar5,uVar8,param_3,param_4,bVar1,bVar2,iVar3,uVar7,uVar6);
      }
    }
  }
  return;
}


