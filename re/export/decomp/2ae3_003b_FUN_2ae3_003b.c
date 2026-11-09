// FUN_2ae3_003b @ 2ae3:003b

bool __cdecl16far FUN_2ae3_003b(undefined2 param_1,undefined2 param_2)

{
  int iVar1;
  bool bVar2;
  
  bVar2 = DAT_2cff_2842 != 0x20;
  if (bVar2) {
    iVar1 = DAT_2cff_2842 * 4;
    *(undefined2 *)(iVar1 + -0x412c) = param_2;
    *(undefined2 *)(iVar1 + -0x412e) = param_1;
    DAT_2cff_2842 = DAT_2cff_2842 + 1;
  }
  return !bVar2;
}


