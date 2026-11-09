// FUN_1353_074e @ 1353:074e

void __cdecl16far
FUN_1353_074e(undefined4 param_1,int param_2,undefined2 param_3,int param_4,undefined4 param_5)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  undefined2 uVar5;
  
  uVar5 = (undefined2)((ulong)param_1 >> 0x10);
  iVar2 = (int)param_1;
  if ((*(int *)(iVar2 + 0xc) != DAT_2cff_3c6a) || (*(int *)(iVar2 + 0xe) != DAT_2cff_3c6e)) {
    FUN_1353_0b64(iVar2,uVar5,*(undefined2 *)(iVar2 + 0xc),*(undefined2 *)(iVar2 + 0xe));
  }
  if (param_4 == 1) {
    iVar3 = 8;
  }
  else if (param_4 == 2) {
    iVar3 = 6;
  }
  else {
    iVar3 = 0;
  }
  if (iVar3 != 0) {
    uVar4 = 0;
    while( true ) {
      uVar1 = FUN_2b9a_000b((int)param_5,param_5._2_2_);
      if (uVar1 <= uVar4) break;
      FUN_1123_00eb(iVar2,uVar5,param_4 * 0x100 + ((int)*(char *)((int)param_5 + uVar4) & 0x7fU),
                    iVar3 * uVar4 + param_2,param_3);
      uVar4 = uVar4 + 1;
    }
  }
  return;
}


