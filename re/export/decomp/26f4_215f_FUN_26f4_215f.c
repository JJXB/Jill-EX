// FUN_26f4_215f @ 26f4:215f

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_26f4_215f(void)

{
  int iVar1;
  undefined2 uVar2;
  
  *(uint *)((int)_DAT_2cff_b7f2 + 8) = DAT_2cff_96ad + DAT_2cff_b998 * -8 & 0xfff8;
  uVar2 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
  iVar1 = (int)_DAT_2cff_b7f2;
  if (*(int *)(iVar1 + 8) < 0) {
    *(undefined2 *)(iVar1 + 8) = 0;
  }
  else if ((0x80 - DAT_2cff_b998) * 0x10 < *(int *)(iVar1 + 8)) {
    *(int *)(iVar1 + 8) = (0x80 - DAT_2cff_b998) * 0x10;
  }
  *(int *)((int)_DAT_2cff_b7f2 + 10) = DAT_2cff_96af + 0x10 + DAT_2cff_b99a * -8;
  uVar2 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
  iVar1 = (int)_DAT_2cff_b7f2;
  if (*(int *)(iVar1 + 10) < 0) {
    *(undefined2 *)(iVar1 + 10) = 0;
  }
  else if ((0x41 - DAT_2cff_b99a) * 0x10 < *(int *)(iVar1 + 10)) {
    *(int *)(iVar1 + 10) = (0x41 - DAT_2cff_b99a) * 0x10;
  }
  DAT_2cff_bec6 = 0;
  DAT_2cff_bec8 = 0;
  return;
}


