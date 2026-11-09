// FUN_2ae9_001f @ 2ae9:001f

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_2ae9_001f(undefined4 param_1)

{
  int iVar1;
  undefined2 uVar2;
  int iVar3;
  int iVar4;
  undefined2 uVar5;
  bool bVar6;
  undefined1 uStack0000;
  
  bVar6 = &uStack0000 == (char *)s_Turbo_C___Copyright__c__1988_Bor_2cff_0004 + 2;
  uVar5 = (undefined2)((ulong)param_1 >> 0x10);
  iVar3 = (int)param_1;
  _DAT_2cff_284e = *(undefined2 *)(iVar3 + 0xe);
  DAT_2cff_284c = *(int *)(iVar3 + 0xc);
  FUN_1000_036f();
  uVar2 = _DAT_2cff_284e;
  iVar1 = DAT_2cff_284c;
  if (bVar6) {
    DAT_2cff_284c = 0;
    _DAT_2cff_284e = 0;
  }
  else {
    iVar4 = (int)*(undefined4 *)(iVar3 + 8);
    uVar5 = (undefined2)((ulong)*(undefined4 *)(iVar3 + 8) >> 0x10);
    *(undefined2 *)(DAT_2cff_284c + 10) = uVar5;
    *(int *)(iVar1 + 8) = iVar4;
    iVar1 = DAT_2cff_284c;
    *(undefined2 *)(iVar4 + 0xe) = _DAT_2cff_284e;
    *(int *)(iVar4 + 0xc) = iVar1;
  }
  return;
}


