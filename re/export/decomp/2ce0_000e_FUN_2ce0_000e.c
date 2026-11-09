// FUN_2ce0_000e @ 2ce0:000e

int __cdecl16far FUN_2ce0_000e(undefined2 param_1,undefined2 param_2)

{
  int iVar1;
  int iVar2;
  int in_DX;
  
  iVar1 = FUN_2b9a_000b(param_1,param_2);
  iVar2 = FUN_2ae9_000a(iVar1 + 1);
  if (in_DX != 0 || iVar2 != 0) {
    FUN_2ba2_0001(iVar2,in_DX,param_1,param_2,iVar1 + 1);
  }
  return iVar2;
}


