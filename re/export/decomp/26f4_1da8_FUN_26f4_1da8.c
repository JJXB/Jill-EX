// FUN_26f4_1da8 @ 26f4:1da8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 __cdecl16far FUN_26f4_1da8(int param_1)

{
  int iVar1;
  undefined2 uVar2;
  
  uVar2 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
  iVar1 = (int)_DAT_2cff_b7f2;
  if ((((*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
         *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) < *(int *)(iVar1 + 8)) ||
       (*(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
        *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f) < *(int *)(iVar1 + 10))) ||
      (*(int *)(iVar1 + 8) + *(int *)(iVar1 + 4) <
       *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f))) ||
     (*(int *)(iVar1 + 10) + *(int *)(iVar1 + 6) <
      *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f))) {
    uVar2 = 0;
  }
  else {
    uVar2 = 1;
  }
  return uVar2;
}


