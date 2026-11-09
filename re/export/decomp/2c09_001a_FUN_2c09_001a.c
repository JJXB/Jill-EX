// FUN_2c09_001a @ 2c09:001a

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_2c09_001a(undefined4 param_1)

{
  undefined2 uVar1;
  int iVar2;
  int iVar3;
  undefined2 uVar4;
  bool bVar5;
  
  bVar5 = true;
  FUN_1000_036f();
  if (bVar5) {
    _DAT_2cff_284c = param_1;
    *(undefined2 *)((int)param_1 + 10) = param_1._2_2_;
    *(int *)((int)param_1 + 8) = (int)param_1;
    *(undefined2 *)((int)param_1 + 0xe) = param_1._2_2_;
    *(int *)((int)param_1 + 0xc) = (int)param_1;
  }
  else {
    uVar4 = (undefined2)((ulong)_DAT_2cff_284c >> 0x10);
    iVar2 = (int)_DAT_2cff_284c;
    iVar3 = (int)*(undefined4 *)(iVar2 + 0xc);
    uVar1 = (undefined2)((ulong)*(undefined4 *)(iVar2 + 0xc) >> 0x10);
    *(undefined2 *)(iVar2 + 0xe) = param_1._2_2_;
    *(int *)(iVar2 + 0xc) = (int)param_1;
    *(undefined2 *)(iVar3 + 10) = param_1._2_2_;
    *(int *)(iVar3 + 8) = (int)param_1;
    *(undefined2 *)((int)param_1 + 0xe) = uVar1;
    *(int *)((int)param_1 + 0xc) = iVar3;
    uVar1 = DAT_2cff_284c;
    *(undefined2 *)((int)param_1 + 10) = _DAT_2cff_284e;
    *(undefined2 *)((int)param_1 + 8) = uVar1;
  }
  return;
}


