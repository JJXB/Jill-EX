// FUN_2c86_0006 @ 2c86:0006

undefined2 __cdecl16far
FUN_2c86_0006(int param_1,int param_2,int param_3,int param_4,int param_5,undefined2 param_6)

{
  int iVar1;
  undefined2 uVar2;
  undefined4 uVar3;
  int iVar4;
  
  iVar1 = FUN_2cc3_0194(param_4,param_3,param_2,param_1);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    iVar1 = (param_3 - param_1) + 1;
    for (; param_2 <= param_4; param_2 = param_2 + 1) {
      iVar4 = param_5;
      uVar2 = param_6;
      uVar3 = FUN_2cea_000e(param_2,param_1);
      FUN_2cc3_0157(iVar1,uVar3,iVar4,uVar2);
      param_5 = param_5 + iVar1 * 2;
    }
    uVar2 = 1;
  }
  return uVar2;
}


