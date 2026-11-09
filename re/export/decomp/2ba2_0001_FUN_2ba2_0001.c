// FUN_2ba2_0001 @ 2ba2:0001

undefined2 * __cdecl16far FUN_2ba2_0001(undefined2 *param_1,undefined2 *param_2,uint param_3)

{
  undefined2 *puVar1;
  undefined2 *puVar2;
  uint uVar3;
  undefined2 *puVar4;
  undefined2 *puVar5;
  undefined2 uVar6;
  undefined2 uVar7;
  
  uVar6 = (undefined2)((ulong)param_1 >> 0x10);
  uVar7 = (undefined2)((ulong)param_2 >> 0x10);
  puVar4 = (undefined2 *)param_2;
  puVar5 = (undefined2 *)param_1;
  for (uVar3 = param_3 >> 1; uVar3 != 0; uVar3 = uVar3 - 1) {
    puVar2 = puVar5;
    puVar5 = puVar5 + 1;
    puVar1 = puVar4;
    puVar4 = puVar4 + 1;
    *puVar2 = *puVar1;
  }
  if ((param_3 & 1) != 0) {
    *(undefined1 *)puVar5 = *(undefined1 *)puVar4;
  }
  return (undefined2 *)param_1;
}


