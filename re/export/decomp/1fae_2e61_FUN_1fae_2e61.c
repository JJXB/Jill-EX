// FUN_1fae_2e61 @ 1fae:2e61

/* WARNING: Restarted to delay deadcode elimination for space: ram */

int __cdecl16far FUN_1fae_2e61(int param_1)

{
  uint *puVar1;
  byte *pbVar2;
  code *pcVar3;
  bool bVar4;
  char cVar5;
  undefined2 uVar6;
  byte bVar7;
  byte bVar11;
  uint uVar8;
  int iVar9;
  int iVar10;
  undefined2 in_CX;
  byte in_DL;
  int *piVar12;
  int iVar13;
  int unaff_DI;
  int iVar14;
  undefined2 unaff_ES;
  undefined2 unaff_SS;
  int iStackY_32;
  int iStackY_2c;
  uint local_4;
  
  iVar13 = DAT_2cff_96b7;
  uVar6 = DAT_2cff_96b5;
  cVar5 = DAT_2cff_96ac;
  bVar4 = false;
  uVar8 = param_1 - 4;
  if (uVar8 < 4) {
    piVar12 = (int *)(uVar8 * 2);
    bVar7 = (byte)uVar8;
    switch(uVar8) {
    case 0:
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      puVar1 = (uint *)((int)register0x00000010 + unaff_DI + -2);
      *puVar1 = *puVar1 + (uint)(0 < (int)-(*puVar1 & 3)) * -(*puVar1 & 3);
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(byte *)piVar12 = (char)*piVar12 + bVar7;
      *(char *)((int)piVar12 + unaff_DI) = *(char *)((int)piVar12 + unaff_DI) + bVar7;
      iVar13 = *piVar12;
      pbVar2 = (byte *)((int)piVar12 + unaff_DI);
      bVar11 = *pbVar2;
      *pbVar2 = *pbVar2 + in_DL;
      *piVar12 = *piVar12 + CONCAT11((char)((uint)in_CX >> 8),(char)in_CX + (char)iVar13) +
                 (uint)(CARRY1(bVar7,*(byte *)((int)piVar12 + unaff_DI)) ||
                       CARRY1(bVar7 + *(byte *)((int)piVar12 + unaff_DI),CARRY1(bVar11,in_DL)));
      pcVar3 = (code *)swi(1);
      iVar13 = (*pcVar3)();
      return iVar13;
    case 1:
      bVar11 = (byte)(uVar8 >> 8);
      iVar13 = CONCAT11(bVar11,bVar7 | bVar11);
      DAT_2cff_b98c = DAT_2cff_96b7 - *(int *)(CONCAT11(0xa1,(char)piVar12) + 10);
      FUN_1123_00eb(0xb986,0x2cff,unaff_DI + 2,
                    *(undefined2 *)((int)(uint *)&DAT_2cff_96ad + iVar13 * 0x1f),
                    (*(int *)((int)(int *)&DAT_2cff_96af + iVar13 * 0x1f) + 0x21) -
                    *(int *)((int)(int *)&DAT_2cff_96bd + iVar13 * 0x1f),0x2cff);
      iVar13 = FUN_1fae_1d0f();
      return iVar13;
    case 3:
      iVar13 = in(0x5d);
      return iVar13;
    }
    if ((piVar12 != (int *)0x0) &&
       (iVar13 = FUN_26f4_2227(0,DAT_2cff_96ad + DAT_2cff_3c76 * 8,DAT_2cff_96af,1), iVar13 != 0)) {
      DAT_2cff_96bf = 0;
      FUN_26f4_1ad4(0,DAT_2cff_96ad + DAT_2cff_3c76 * 8,DAT_2cff_96af);
      DAT_2cff_96b9 = 2;
      DAT_2cff_96bb = 2;
      DAT_2cff_96b1 = DAT_2cff_3c76;
      if (DAT_2cff_3c7a == 0) {
        DAT_2cff_96b3 = 0xfffc;
      }
      else {
        DAT_2cff_3d9e = 1;
        DAT_2cff_96b3 = 0xfff4;
        FUN_1581_0178(1,1);
      }
    }
    if (DAT_2cff_96b1 != 0) {
      DAT_2cff_96c7 = DAT_2cff_96b1;
    }
    if (((DAT_2cff_3c7e != 0) && ((((undefined2 *)&DAT_2cff_be2c)[DAT_2cff_96b9] & 1) != 0)) &&
       (DAT_2cff_3da0 = 1, DAT_2cff_96c7 != 0)) {
      iStackY_32 = 0;
      for (iStackY_2c = 0; iStackY_2c < DAT_2cff_becc; iStackY_2c = iStackY_2c + 1) {
        iStackY_32 = iStackY_32 +
                     (uint)(((char *)&DAT_2cff_96ac)
                            [((undefined2 *)&DAT_2cff_bcac)[iStackY_2c] * 0x1f] == '2');
      }
      iVar13 = FUN_26f4_1a89(8);
      if (iStackY_32 < iVar13) {
        FUN_26f4_1892(0x32,DAT_2cff_96ad + DAT_2cff_96c7 * 0xc + 4,DAT_2cff_96af + 2);
        *(int *)((int)(int *)&DAT_2cff_96b1 + (DAT_2cff_bb1c + -1) * 0x1f) = DAT_2cff_96c7 * 6;
        FUN_1581_0178(2,0x22);
      }
      else {
        iVar13 = FUN_26f4_1a38(2);
        if (iVar13 == 0) {
          FUN_1581_0178(2,8);
        }
        else {
          iVar13 = FUN_26f4_1892(2,DAT_2cff_96ad + DAT_2cff_96c7 * 0xc + 4,DAT_2cff_96af + 2);
          if (iVar13 != 0) {
            iVar10 = FUN_26f4_2227(iVar13,*(undefined2 *)
                                           ((int)(uint *)&DAT_2cff_96ad + iVar13 * 0x1f),
                                   *(undefined2 *)((int)(int *)&DAT_2cff_96af + iVar13 * 0x1f),1);
            if (iVar10 == 1) {
              *(int *)((int)(int *)&DAT_2cff_96b1 + (DAT_2cff_bb1c + -1) * 0x1f) =
                   DAT_2cff_96c7 << 3;
              *(undefined2 *)((int)(int *)&DAT_2cff_96bd + (DAT_2cff_bb1c + -1) * 0x1f) = 3;
              FUN_1581_0178(2,3);
            }
            else {
              FUN_26f4_1863(iVar13);
              FUN_26f4_1a13(2);
              FUN_1581_0178(2,8);
            }
          }
        }
      }
    }
    DAT_2cff_96bd = DAT_2cff_96bd + 1;
    if (DAT_2cff_96bf != 0) {
      DAT_2cff_96bf = DAT_2cff_96bf + -1;
    }
    FUN_26f4_23e1(0);
    FUN_2cff_2afd((char *)s_JILL_SPLASHES_INTO_THE_UNDERGROU_2cff_26ea + 10,local_4);
    iVar13 = FUN_1fae_1d0f();
    return iVar13;
  }
  if (DAT_2cff_96ac == '\0') {
    iVar10 = 0;
    DAT_2cff_96ac = cVar5;
    DAT_2cff_96b5 = uVar6;
    DAT_2cff_96b7 = iVar13;
  }
  else {
    DAT_2cff_96ac = '\0';
    DAT_2cff_96b5 = DAT_2cff_b6b6;
    uVar8 = DAT_2cff_96ad & 0xfff8;
    iVar14 = (DAT_2cff_96af + DAT_2cff_96b7) - DAT_2cff_b740;
    DAT_2cff_96b7 = DAT_2cff_b740;
    iVar9 = FUN_26f4_2227(0,uVar8,iVar14,1);
    iVar10 = DAT_2cff_96af;
    if (iVar9 == 0) {
      iVar9 = FUN_26f4_2227(0,uVar8,DAT_2cff_96af,1);
      if (iVar9 != 0) {
        bVar4 = true;
      }
    }
    else {
      bVar4 = true;
      iVar10 = iVar14;
    }
    if (bVar4) {
      FUN_26f4_1a13(param_1);
      DAT_2cff_96b9 = 0;
      DAT_2cff_96bb = 0;
      DAT_2cff_96bf = 0;
      DAT_2cff_96b1 = 0;
      DAT_2cff_96b3 = 0;
      DAT_2cff_96ad = uVar8;
      DAT_2cff_96af = iVar10;
      for (iVar13 = 0; iVar13 < 0xb; iVar13 = iVar13 + 1) {
        iVar10 = *(int *)(iVar13 * 2 + 0xeea);
        while (iVar10 != 0) {
          iVar10 = FUN_26f4_1a38(iVar13);
        }
      }
      FUN_1a41_09a8(DAT_2cff_96ad,DAT_2cff_96af,10);
      iVar10 = FUN_22ab_05b4(*(undefined2 *)(param_1 * 4 + 0xf00),
                             *(undefined2 *)(param_1 * 4 + 0xf02),7);
    }
    else {
      iVar10 = iVar13;
      DAT_2cff_96ac = cVar5;
      DAT_2cff_96b5 = uVar6;
      DAT_2cff_96b7 = iVar13;
    }
  }
  return iVar10;
}


