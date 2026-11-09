// FUN_1353_09dc @ 1353:09dc

void __cdecl16far
FUN_1353_09dc(undefined4 param_1,undefined2 param_2,int param_3,undefined2 param_4,
             undefined2 param_5)

{
  int iVar1;
  int iVar2;
  undefined2 uVar3;
  
  if (param_3 == 1) {
    iVar2 = 8;
  }
  else if (param_3 == 2) {
    iVar2 = 6;
  }
  else {
    iVar2 = 0;
  }
  iVar1 = FUN_2b9a_000b(param_4,param_5,param_2,param_3,param_4,param_5);
  uVar3 = (undefined2)((ulong)param_1 >> 0x10);
  FUN_1353_074e((int)param_1,uVar3,*(int *)((int)param_1 + 4) - ((uint)(iVar1 * iVar2) >> 1));
  return;
}


