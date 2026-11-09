// FUN_26f4_00f2 @ 26f4:00f2

int __cdecl16far FUN_26f4_00f2(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = 0;
  for (iVar1 = 0; iVar1 < DAT_2cff_bb1c; iVar1 = iVar1 + 1) {
    iVar2 = iVar2 + (uint)((char)((undefined1 *)&DAT_2cff_96ac)[iVar1 * 0x1f] == param_1);
  }
  return iVar2;
}


