// FUN_1123_081b @ 1123:081b

/* WARNING: Removing unreachable block (ram,0x00011a74) */
/* WARNING: Removing unreachable block (ram,0x00011a68) */
/* WARNING: Removing unreachable block (ram,0x00011a88) */
/* WARNING: Removing unreachable block (ram,0x00011a94) */

void __cdecl16far FUN_1123_081b(undefined4 param_1)

{
  undefined1 uVar1;
  int iVar2;
  undefined2 uVar3;
  uint local_4;
  
  if (DAT_2cff_3a4d == '\x04') {
    for (local_4 = 0; local_4 < 0x100; local_4 = local_4 + 1) {
      out(DAT_2cff_0496,(undefined1)local_4);
      uVar1 = in(DAT_2cff_0498);
      uVar3 = (undefined2)((ulong)param_1 >> 0x10);
      iVar2 = (int)param_1;
      *(undefined1 *)(iVar2 + local_4 * 3) = uVar1;
      uVar1 = in(DAT_2cff_0498);
      *(undefined1 *)(iVar2 + local_4 * 3 + 1) = uVar1;
      uVar1 = in(DAT_2cff_0498);
      *(undefined1 *)(iVar2 + local_4 * 3 + 2) = uVar1;
    }
  }
  return;
}


