// FUN_1353_0b20 @ 1353:0b20

void __cdecl16far FUN_1353_0b20(undefined4 param_1,undefined2 param_2)

{
  int iVar1;
  undefined2 uVar2;
  
  uVar2 = (undefined2)((ulong)param_1 >> 0x10);
  iVar1 = (int)param_1;
  *(undefined2 *)(iVar1 + 8) = 0;
  *(undefined2 *)(iVar1 + 10) = 0;
  *(undefined2 *)(iVar1 + 0xc) = 1;
  *(undefined2 *)(iVar1 + 0xe) = param_2;
  return;
}


