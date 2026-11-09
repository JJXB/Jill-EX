// FUN_26f4_11d7 @ 26f4:11d7

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_26f4_11d7(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  undefined2 uVar7;
  
  uVar7 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
  iVar1 = *(int *)((int)_DAT_2cff_b7f2 + 8) >> 4;
  iVar2 = *(int *)((int)_DAT_2cff_b7f2 + 10) >> 4;
  if (iVar1 + DAT_2cff_b998 < 0x7f) {
    iVar3 = iVar1 + DAT_2cff_b998;
  }
  else {
    iVar3 = 0x7f;
  }
  if (iVar2 + DAT_2cff_b99a < 0x3f) {
    iVar4 = iVar2 + DAT_2cff_b99a;
  }
  else {
    iVar4 = 0x3f;
  }
  for (; iVar1 <= iVar3; iVar1 = iVar1 + 1) {
    for (iVar6 = iVar2; iVar6 <= iVar4; iVar6 = iVar6 + 1) {
      if ((*(uint *)(((undefined2 *)&DAT_2cff_43a6)[iVar1 * 0x40 + iVar6] * 8 + -0x7c12) & 0x20) !=
          0) {
        iVar5 = FUN_1f28_0006(iVar1,iVar6,2);
        ((undefined2 *)&DAT_2cff_43a6)[iVar1 * 0x40 + iVar6] =
             ((undefined2 *)&DAT_2cff_43a6)[iVar1 * 0x40 + iVar6] | (uint)(iVar5 != 0) * -0x4000;
      }
    }
  }
  return;
}


