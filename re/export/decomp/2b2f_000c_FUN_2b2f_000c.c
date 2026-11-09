// FUN_2b2f_000c @ 2b2f:000c

undefined2 __cdecl16far FUN_2b2f_000c(undefined2 param_1,undefined2 param_2)

{
  code *pcVar1;
  undefined2 uVar2;
  undefined1 in_CF;
  
  pcVar1 = (code *)swi(0x21);
  uVar2 = (*pcVar1)();
  if ((bool)in_CF) {
    FUN_2adf_000c(uVar2);
  }
  else {
    param_2 = 0xffff;
  }
  return param_2;
}


