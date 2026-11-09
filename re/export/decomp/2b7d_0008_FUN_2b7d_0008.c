// FUN_2b7d_0008 @ 2b7d:0008

undefined2 __cdecl16far FUN_2b7d_0008(int param_1)

{
  uint *puVar1;
  code *pcVar2;
  undefined2 uVar3;
  undefined1 uVar4;
  undefined2 in_stack_00000000;
  
  uVar4 = 0;
  if ((*(uint *)(param_1 * 2 + 0x2954) & 0x800) != 0) {
    uVar4 = 0;
    FUN_2b82_0004(param_1,0,0,2);
  }
  pcVar2 = (code *)swi(0x21);
  uVar3 = (*pcVar2)();
  if ((bool)uVar4) {
    uVar3 = FUN_2adf_000c();
  }
  else {
    puVar1 = (uint *)(param_1 * 2 + 0x2954);
    *puVar1 = *puVar1 | 0x1000;
  }
  return uVar3;
}


