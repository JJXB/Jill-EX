// FUN_22ab_1202 @ 22ab:1202

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_22ab_1202(void)

{
  int iVar1;
  int iVar2;
  
  for (iVar1 = 0; iVar1 < 0x80; iVar1 = iVar1 + 1) {
    for (iVar2 = 0; iVar2 < 0x40; iVar2 = iVar2 + 1) {
      ((undefined2 *)&DAT_2cff_43a6)[iVar1 * 0x40 + iVar2] = 0xc000;
    }
  }
  *(undefined2 *)((int)_DAT_2cff_b7f2 + 8) = 0;
  *(undefined2 *)((int)_DAT_2cff_b7f2 + 10) = 0;
  return;
}


