// FUN_1353_0a32 @ 1353:0a32

void __cdecl16far FUN_1353_0a32(undefined4 param_1,undefined2 param_2,undefined2 param_3)

{
  int iVar1;
  int iVar2;
  undefined2 uVar3;
  
  iVar1 = FUN_2b9a_000b(param_2,param_3,4,1,param_2,param_3);
  uVar3 = (undefined2)((ulong)param_1 >> 0x10);
  iVar2 = (int)param_1;
  FUN_1353_074e(iVar2 + 0x18,uVar3,
                (*(int *)(iVar2 + 8) + *(int *)(iVar2 + 0x10)) / 2 + 0x10 + iVar1 * -4);
  return;
}


