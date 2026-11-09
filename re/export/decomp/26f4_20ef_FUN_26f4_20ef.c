// FUN_26f4_20ef @ 26f4:20ef

void __cdecl16far FUN_26f4_20ef(int param_1,undefined2 param_2,undefined2 param_3)

{
  int iVar1;
  
  for (iVar1 = 0; iVar1 < DAT_2cff_bb1c; iVar1 = iVar1 + 1) {
    if (((((undefined2 *)&DAT_2cff_be3c)[(char)((undefined1 *)&DAT_2cff_96ac)[iVar1 * 0x1f]] & 0x100
         ) != 0) && (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + iVar1 * 0x1f) == param_1)) {
      (*(code *)((undefined2 *)&DAT_2cff_b84e)
                [(char)((undefined1 *)&DAT_2cff_96ac)[iVar1 * 0x1f] * 2])
                ((char *)s_JILL_SPLASHES_INTO_THE_UNDERGROU_2cff_26ea + 10,iVar1,param_2,param_3);
    }
  }
  return;
}


