// FUN_2c9a_0000 @ 2c9a:0000

undefined2 __cdecl16far
FUN_2c9a_0000(int param_1,int param_2,int param_3,int param_4,int param_5,int param_6)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int local_8;
  int local_6;
  int local_4;
  
  iVar1 = FUN_2cc3_0194(param_4,param_3,param_2,param_1);
  if ((iVar1 == 0) ||
     (iVar1 = FUN_2cc3_0194((param_4 - param_2) + param_6,(param_3 - param_1) + param_5,param_6,
                            param_5), iVar1 == 0)) {
    return 0;
  }
  local_8 = param_2;
  local_6 = param_4;
  local_4 = 1;
  if (param_2 < param_6) {
    local_8 = param_4;
    local_6 = param_2;
    local_4 = -1;
  }
  for (; local_6 + local_4 != local_8; local_8 = local_8 + local_4) {
    uVar2 = FUN_2cea_000e((local_8 - param_2) + param_6,param_5);
    uVar3 = FUN_2cea_000e(local_8,param_1);
    FUN_2cc3_0157((param_3 - param_1) + 1,uVar3,uVar2);
  }
  return 1;
}


