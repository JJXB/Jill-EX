// FUN_22ab_0e86 @ 22ab:0e86

void __cdecl16far FUN_22ab_0e86(undefined4 param_1,undefined2 param_2,undefined2 param_3)

{
  undefined2 uVar1;
  int iVar2;
  undefined1 *puVar3;
  undefined2 uVar4;
  undefined1 local_52 [80];
  
  uVar4 = 1;
  uVar1 = FUN_22ab_0dc6(param_3,local_52);
  FUN_1353_0b64((int)param_1,param_1._2_2_,uVar1,uVar4);
  FUN_1353_074e((int)param_1,param_1._2_2_,0,param_2,2,0x1342,0x2cff);
  puVar3 = local_52;
  uVar1 = 2;
  iVar2 = FUN_2b9a_000b(local_52);
  FUN_1353_074e((int)param_1,param_1._2_2_,(uint)(*(int *)((int)param_1 + 4) + iVar2 * -6) >> 1,
                param_2,uVar1,puVar3);
  return;
}


