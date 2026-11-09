// FUN_26f4_0d52 @ 26f4:0d52

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_26f4_0d52(int param_1)

{
  int *piVar1;
  uint *puVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  undefined2 uVar9;
  char *pcVar10;
  undefined2 unaff_SS;
  int local_a0e;
  int local_a0c;
  byte local_a02 [2560];
  
  pcVar10 = (char *)s_JILL_SPLASHES_INTO_THE_UNDERGROU_2cff_26ea + 10;
  if (param_1 == 0) {
    if (DAT_2cff_be38 != 0) {
      pcVar10 = (char *)0x22ab;
      FUN_22ab_05dd();
      DAT_2cff_be38 = 0;
    }
    for (iVar7 = 0; iVar7 < 0x80; iVar7 = iVar7 + 1) {
      local_a02[iVar7 * 0x14] = 0xff;
    }
    if ((DAT_2cff_bab0 != 0) || (DAT_2cff_bab2 != 0)) {
      FUN_26f4_09fb(DAT_2cff_bab0,DAT_2cff_bab2);
    }
    uVar9 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
    iVar5 = (int)_DAT_2cff_b7f2;
    iVar7 = (*(int *)(iVar5 + 8) >> 4) + DAT_2cff_b998;
    iVar3 = (*(int *)(iVar5 + 10) >> 4) + DAT_2cff_b99a;
    if (*(int *)(iVar5 + 8) >> 4 < 2) {
      iVar4 = 0;
    }
    else {
      iVar4 = (*(int *)(iVar5 + 8) >> 4) + -2;
    }
    if (*(int *)(iVar5 + 10) >> 4 < 2) {
      iVar5 = 0;
    }
    else {
      iVar5 = (*(int *)(iVar5 + 10) >> 4) + -2;
    }
    for (local_a0c = 0; local_a0c < DAT_2cff_bb1c; local_a0c = local_a0c + 1) {
      if ((*(uint *)(local_a0c * 0x1f + -0x693f) & 0xc000) != 0) {
        iVar6 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + local_a0c * 0x1f) >> 4;
        if (iVar6 < iVar4) {
          iVar6 = iVar4;
        }
        iVar8 = 0;
        while (local_a02[iVar8 + iVar6 * 0x14] != 0xff) {
          iVar8 = iVar8 + 1;
        }
        local_a02[iVar8 + iVar6 * 0x14] = (byte)local_a0c;
        local_a02[iVar8 + iVar6 * 0x14 + 1] = 0xff;
        puVar2 = (uint *)(local_a0c * 0x1f + -0x693f);
        *puVar2 = *puVar2 & 0x3fff;
      }
    }
    while( true ) {
      iVar7 = iVar7 + -1;
      local_a0e = iVar3;
      if (iVar7 < iVar4) break;
      while (local_a0e = local_a0e + -1, iVar5 <= local_a0e) {
        if ((((undefined2 *)&DAT_2cff_43a6)[iVar7 * 0x40 + local_a0e] & 0x8000) != 0) {
          pcVar10 = (char *)0x22ab;
          FUN_22ab_140b(iVar7,local_a0e);
          ((undefined2 *)&DAT_2cff_43a6)[iVar7 * 0x40 + local_a0e] =
               ((undefined2 *)&DAT_2cff_43a6)[iVar7 * 0x40 + local_a0e] & 0x3fff;
        }
      }
      for (iVar6 = 0; (local_a02[iVar6 + iVar7 * 0x14] != 0xff && (iVar6 < 0x14)); iVar6 = iVar6 + 1
          ) {
        (*(code *)((undefined2 *)&DAT_2cff_b84e)
                  [(char)((undefined1 *)&DAT_2cff_96ac)
                         [(uint)local_a02[iVar6 + iVar7 * 0x14] * 0x1f] * 2])
                  (pcVar10,(uint)local_a02[iVar6 + iVar7 * 0x14],0,0);
      }
    }
  }
  else {
    if (DAT_2cff_be38 != 0) {
      pcVar10 = (char *)0x22ab;
      FUN_22ab_05dd();
      DAT_2cff_be38 = DAT_2cff_be38 & (DAT_2cff_3a50 + 1) * 0x4000;
    }
    if ((DAT_2cff_bab0 + DAT_2cff_bec6 != 0) || (DAT_2cff_bab2 + DAT_2cff_bec8 != 0)) {
      piVar1 = (int *)((int)_DAT_2cff_b7f2 + 8);
      *piVar1 = *piVar1 - DAT_2cff_bec6;
      piVar1 = (int *)((int)_DAT_2cff_b7f2 + 10);
      *piVar1 = *piVar1 - DAT_2cff_bec8;
      FUN_26f4_0d0c(DAT_2cff_bab0 + DAT_2cff_bec6,DAT_2cff_bab2 + DAT_2cff_bec8);
    }
    DAT_2cff_bec6 = DAT_2cff_bab0;
    DAT_2cff_bec8 = DAT_2cff_bab2;
    uVar9 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
    iVar7 = (int)_DAT_2cff_b7f2;
    if ((*(int *)(iVar7 + 8) >> 4) + DAT_2cff_b998 < 0x7f) {
      iVar3 = (*(int *)(iVar7 + 8) >> 4) + DAT_2cff_b998;
    }
    else {
      iVar3 = 0x7f;
    }
    if ((*(int *)(iVar7 + 10) >> 4) + DAT_2cff_b99a + -1 < 0x3f) {
      iVar5 = (*(int *)(iVar7 + 10) >> 4) + DAT_2cff_b99a + -1;
    }
    else {
      iVar5 = 0x3f;
    }
    if (*(int *)(iVar7 + 8) >> 4 < 2) {
      iVar4 = 0;
    }
    else {
      iVar4 = (*(int *)(iVar7 + 8) >> 4) + -2;
    }
    if (*(int *)(iVar7 + 10) >> 4 < 2) {
      iVar7 = 0;
    }
    else {
      iVar7 = (*(int *)(iVar7 + 10) >> 4) + -2;
    }
    for (; local_a0c = DAT_2cff_bb1c, iVar4 <= iVar3; iVar3 = iVar3 + -1) {
      for (local_a0e = iVar5; iVar7 <= local_a0e; local_a0e = local_a0e + -1) {
        if ((((undefined2 *)&DAT_2cff_43a6)[iVar3 * 0x40 + local_a0e] & 0xc000) != 0) {
          pcVar10 = (char *)0x22ab;
          FUN_22ab_140b(iVar3,local_a0e);
          ((undefined2 *)&DAT_2cff_43a6)[iVar3 * 0x40 + local_a0e] =
               ((undefined2 *)&DAT_2cff_43a6)[iVar3 * 0x40 + local_a0e] &
               ((DAT_2cff_3a50 + 1) * 0x4000 ^ 0xffffU);
        }
      }
    }
    while (local_a0c = local_a0c + -1, -1 < local_a0c) {
      if ((*(uint *)(local_a0c * 0x1f + -0x693f) & 0xc000) != 0) {
        (*(code *)((undefined2 *)&DAT_2cff_b84e)
                  [(char)((undefined1 *)&DAT_2cff_96ac)[local_a0c * 0x1f] * 2])
                  (pcVar10,local_a0c,0,0);
        puVar2 = (uint *)(local_a0c * 0x1f + -0x693f);
        *puVar2 = *puVar2 & ((DAT_2cff_3a50 + 1) * 0x4000 ^ 0xffffU);
      }
    }
    FUN_1123_0716();
  }
  if (DAT_2cff_83d0 != 0) {
    DAT_2cff_83d0 = 0;
    DAT_2cff_be38 = DAT_2cff_be38 | 0xc000;
  }
  return;
}


