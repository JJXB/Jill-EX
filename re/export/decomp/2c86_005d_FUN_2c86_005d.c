// FUN_2c86_005d @ 2c86:005d

undefined2 __cdecl16far
FUN_2c86_005d(int param_1,int param_2,int param_3,int param_4,int param_5,undefined2 param_6)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = (param_3 - param_1) + 1;
  for (; param_2 <= param_4; param_2 = param_2 + 1) {
    uVar2 = FUN_2cea_000e(param_2,param_1);
    FUN_2cc3_0157(iVar1,param_5,param_6,uVar2);
    param_5 = param_5 + iVar1 * 2;
  }
  return 1;
}


