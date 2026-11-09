// FUN_26f4_12b9 @ 26f4:12b9

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_26f4_12b9(int param_1)

{
  uint *puVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  bool bVar8;
  int iVar9;
  int iVar10;
  undefined2 uVar11;
  int local_1c;
  int local_1a;
  int local_e;
  int local_c;
  
  DAT_2cff_becc = 1;
  DAT_2cff_bcac = 0;
  uVar11 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
  iVar9 = (int)_DAT_2cff_b7f2;
  iVar3 = *(int *)(iVar9 + 8);
  iVar4 = *(int *)(iVar9 + 8);
  iVar5 = *(int *)(iVar9 + 4);
  iVar6 = *(int *)(iVar9 + 10);
  iVar7 = *(int *)(iVar9 + 10);
  iVar9 = *(int *)(iVar9 + 6);
  iVar10 = 1;
  while ((iVar10 < DAT_2cff_bb1c && (DAT_2cff_becc < 0xc0))) {
    if ((iVar3 + -0x60 <=
         *(int *)((int)(undefined2 *)&DAT_2cff_96ad + iVar10 * 0x1f) +
         *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + iVar10 * 0x1f)) &&
       (((*(int *)((int)(undefined2 *)&DAT_2cff_96ad + iVar10 * 0x1f) <= iVar4 + iVar5 + 0x60 &&
         (iVar6 + -0x30 <=
          *(int *)((int)(undefined2 *)&DAT_2cff_96af + iVar10 * 0x1f) +
          *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + iVar10 * 0x1f))) &&
        (*(int *)((int)(undefined2 *)&DAT_2cff_96af + iVar10 * 0x1f) <= iVar7 + iVar9 + 0x30)))) {
      ((undefined2 *)&DAT_2cff_bcac)[DAT_2cff_becc] = iVar10;
      DAT_2cff_becc = DAT_2cff_becc + 1;
      puVar1 = (uint *)(iVar10 * 0x1f + -0x693f);
      *puVar1 = *puVar1 & (DAT_2cff_3a50 << 0xe ^ 0xffffU);
    }
    iVar10 = iVar10 + 1;
  }
  DAT_2cff_bab0 = 0;
  DAT_2cff_bab2 = 0;
  DAT_2cff_43a2 = DAT_2cff_96ad;
  DAT_2cff_43a4 = DAT_2cff_96af;
  for (local_1c = 0; local_1c < DAT_2cff_becc; local_1c = local_1c + 1) {
    iVar3 = ((undefined2 *)&DAT_2cff_bcac)[local_1c];
    iVar4 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + iVar3 * 0x1f);
    iVar5 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + iVar3 * 0x1f);
    iVar6 = *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + iVar3 * 0x1f);
    iVar7 = *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + iVar3 * 0x1f);
    if (param_1 == 0) {
      puVar1 = (uint *)(iVar3 * 0x1f + -0x693f);
      *puVar1 = *puVar1 | 0xc000;
    }
    else {
      if (0 < *(int *)((int)(undefined2 *)&DAT_2cff_96c9 + iVar3 * 0x1f)) {
        piVar2 = (int *)((int)(undefined2 *)&DAT_2cff_96c9 + iVar3 * 0x1f);
        *piVar2 = *piVar2 + -1;
      }
      iVar9 = (*(code *)((undefined2 *)&DAT_2cff_b84e)
                        [(char)((undefined1 *)&DAT_2cff_96ac)[iVar3 * 0x1f] * 2])
                        ((char *)s_JILL_SPLASHES_INTO_THE_UNDERGROU_2cff_26ea + 10,iVar3,2,0);
      if (iVar9 != 0) {
        puVar1 = (uint *)(iVar3 * 0x1f + -0x693f);
        *puVar1 = *puVar1 | 0xc000;
      }
    }
    if ((((undefined2 *)&DAT_2cff_be3c)[(char)((undefined1 *)&DAT_2cff_96ac)[iVar3 * 0x1f]] & 8) !=
        0) {
      for (local_1a = 0; local_1a <= DAT_2cff_becc; local_1a = local_1a + 1) {
        iVar9 = ((undefined2 *)&DAT_2cff_bcac)[local_1a];
        if (((iVar9 != iVar3) &&
            (*(int *)((int)(undefined2 *)&DAT_2cff_96ad + iVar9 * 0x1f) <
             *(int *)((int)(undefined2 *)&DAT_2cff_96ad + iVar3 * 0x1f) +
             *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + iVar3 * 0x1f))) &&
           ((*(int *)((int)(undefined2 *)&DAT_2cff_96ad + iVar3 * 0x1f) <
             *(int *)((int)(undefined2 *)&DAT_2cff_96ad + iVar9 * 0x1f) +
             *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + iVar9 * 0x1f) &&
            ((*(int *)((int)(undefined2 *)&DAT_2cff_96af + iVar9 * 0x1f) <
              *(int *)((int)(undefined2 *)&DAT_2cff_96af + iVar3 * 0x1f) +
              *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + iVar3 * 0x1f) &&
             (*(int *)((int)(undefined2 *)&DAT_2cff_96af + iVar3 * 0x1f) <
              *(int *)((int)(undefined2 *)&DAT_2cff_96af + iVar9 * 0x1f) +
              *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + iVar9 * 0x1f))))))) {
          (*(code *)((undefined2 *)&DAT_2cff_b84e)
                    [(char)((undefined1 *)&DAT_2cff_96ac)[iVar3 * 0x1f] * 2])
                    ((char *)s_JILL_SPLASHES_INTO_THE_UNDERGROU_2cff_26ea + 10,iVar3,1,iVar9);
          puVar1 = (uint *)(iVar3 * 0x1f + -0x693f);
          *puVar1 = *puVar1 | 0xc000;
          (*(code *)((undefined2 *)&DAT_2cff_b84e)
                    [(char)((undefined1 *)&DAT_2cff_96ac)[iVar9 * 0x1f] * 2])
                    ((char *)s_JILL_SPLASHES_INTO_THE_UNDERGROU_2cff_26ea + 10,iVar9,1,iVar3);
          puVar1 = (uint *)(iVar9 * 0x1f + -0x693f);
          *puVar1 = *puVar1 | 0xc000;
        }
      }
    }
    if ((*(uint *)(iVar3 * 0x1f + -0x693f) & 0xc000) != 0) {
      for (local_c = iVar5 >> 4; local_c < iVar5 + iVar7 + 0xf >> 4; local_c = local_c + 1) {
        for (local_e = iVar4 >> 4; local_e < iVar4 + iVar6 + 0xf >> 4; local_e = local_e + 1) {
          ((undefined2 *)&DAT_2cff_43a6)[local_e * 0x40 + local_c] =
               ((undefined2 *)&DAT_2cff_43a6)[local_e * 0x40 + local_c] | 0xc000;
        }
      }
    }
  }
  for (local_1c = 0; local_1c < DAT_2cff_becc; local_1c = local_1c + 1) {
    iVar3 = ((undefined2 *)&DAT_2cff_bcac)[local_1c];
    iVar4 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + iVar3 * 0x1f);
    iVar5 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + iVar3 * 0x1f);
    bVar8 = false;
    for (local_c = iVar5 >> 4;
        local_c < *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + iVar3 * 0x1f) + iVar5 + 0xf >> 4;
        local_c = local_c + 1) {
      for (local_e = iVar4 >> 4;
          local_e < *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + iVar3 * 0x1f) + iVar4 + 0xf >> 4;
          local_e = local_e + 1) {
        if ((bVar8) || ((((undefined2 *)&DAT_2cff_43a6)[local_e * 0x40 + local_c] & 0xc000) != 0)) {
          bVar8 = true;
        }
        else {
          bVar8 = false;
        }
      }
    }
    if (bVar8) {
      puVar1 = (uint *)(iVar3 * 0x1f + -0x693f);
      *puVar1 = *puVar1 | 0xc000;
    }
  }
  return;
}


