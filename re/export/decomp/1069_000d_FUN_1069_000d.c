// FUN_1069_000d @ 1069:000d

void __cdecl16far FUN_1069_000d(undefined2 param_1,undefined2 param_2)

{
  int iVar1;
  
  FUN_2b9f_0008(0x2f5c,0x2cff,param_1,param_2);
  for (iVar1 = 0; iVar1 < 0x40; iVar1 = iVar1 + 1) {
    ((undefined2 *)&DAT_2cff_2fae)[iVar1] = 0;
    ((undefined2 *)&DAT_2cff_3130)[iVar1 * 2] = 0;
    ((undefined2 *)&DAT_2cff_312e)[iVar1 * 2] = 0;
  }
  return;
}


