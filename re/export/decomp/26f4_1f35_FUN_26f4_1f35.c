// FUN_26f4_1f35 @ 26f4:1f35

undefined2 __cdecl16far FUN_26f4_1f35(int param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar3 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) + param_2;
  iVar1 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + param_3;
  iVar2 = FUN_26f4_1b8f(param_1,param_2,param_3);
  if ((iVar2 == 0) || (iVar2 = FUN_26f4_2227(param_1,iVar3,iVar1,1), iVar2 != 1)) {
    return 0;
  }
  FUN_26f4_1ad4(param_1,iVar3,iVar1);
  return 1;
}


