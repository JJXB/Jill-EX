// FUN_22ab_0d99 @ 22ab:0d99

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __cdecl16far FUN_22ab_0d99(void)

{
  int iVar1;
  int iVar2;
  
  iVar2 = 0;
  for (iVar1 = 0; iVar1 < DAT_2cff_439a; iVar1 = iVar1 + 1) {
    iVar2 = iVar2 + (uint)(*(char *)((int)_DAT_2cff_3f0a + iVar1) == '\r');
  }
  return iVar2;
}


