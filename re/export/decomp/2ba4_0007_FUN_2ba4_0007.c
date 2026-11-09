// FUN_2ba4_0007 @ 2ba4:0007

void __cdecl16far FUN_2ba4_0007(undefined2 *param_1,uint param_2,undefined1 param_3)

{
  undefined2 *puVar1;
  uint uVar2;
  undefined2 *puVar3;
  undefined2 uVar4;
  
  uVar4 = (undefined2)((ulong)param_1 >> 0x10);
  puVar3 = (undefined2 *)param_1;
  if (((ulong)param_1 & 1) != 0) {
    if (param_2 == 0) {
      return;
    }
    puVar3 = (undefined2 *)((int)puVar3 + 1);
    *(undefined1 *)param_1 = param_3;
    param_2 = param_2 - 1;
  }
  for (uVar2 = param_2 >> 1; uVar2 != 0; uVar2 = uVar2 - 1) {
    puVar1 = puVar3;
    puVar3 = puVar3 + 1;
    *puVar1 = CONCAT11(param_3,param_3);
  }
  if ((param_2 & 1) != 0) {
    *(undefined1 *)puVar3 = param_3;
  }
  return;
}


