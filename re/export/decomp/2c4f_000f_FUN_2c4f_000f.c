// FUN_2c4f_000f @ 2c4f:000f

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Instruction at (ram,0x0002eb45) overlaps instruction at (ram,0x0002eb44)
    */

uint __stdcall16far FUN_2c4f_000f(int param_1,undefined2 param_2,int param_3,byte *param_4)

{
  uint *puVar1;
  int *piVar2;
  char *pcVar3;
  byte *pbVar4;
  undefined1 uVar5;
  char cVar6;
  long lVar7;
  byte bVar8;
  int iVar9;
  int iVar10;
  undefined1 *puVar11;
  int iVar12;
  uint uVar13;
  uint uVar14;
  int in_DX;
  int extraout_DX;
  int extraout_DX_00;
  int unaff_BP;
  uint unaff_SI;
  undefined2 uVar15;
  undefined2 unaff_SS;
  bool bVar16;
  undefined4 uVar17;
  undefined2 in_stack_00000000;
  undefined2 in_stack_00000002;
  int iStackY_32;
  int iStackY_2c;
  uint uStackY_1c;
  int iStackY_1a;
  int iVar18;
  int in_stack_0000ffec;
  int iVar19;
  int iVar20;
  byte in_stack_0000fffa;
  uint uVar21;
  uint local_4;
  
  uVar21 = (uint)in_stack_0000fffa;
  bVar8 = FUN_2cf4_000f();
  puVar11 = (undefined1 *)(uint)bVar8;
  uVar13 = FUN_2cf4_000f();
  uVar13 = uVar13 >> 8;
  while( true ) {
    pbVar4 = param_4;
    iVar10 = param_3 + -1;
    if (param_3 == 0) {
      FUN_1000_0414();
      return uVar21 >> 8;
    }
    uVar15 = (undefined2)((ulong)param_4 >> 0x10);
    param_4 = (byte *)CONCAT22(uVar15,(byte *)param_4 + 1);
    bVar8 = *pbVar4;
    uVar21 = CONCAT11(bVar8,(char)uVar21);
    uVar14 = bVar8 - 7;
    if (uVar14 < 7) break;
    if ((DAT_2cff_2a33 == '\0') && (DAT_2cff_2a39 != 0)) {
      local_4 = CONCAT11(DAT_2cff_2a2e,bVar8);
      uVar17 = FUN_2cea_000e(uVar13 + 1,puVar11 + 1);
      in_stack_0000ffec = 1;
      FUN_2cea_0033(1,&local_4,unaff_SS,uVar17);
      in_DX = extraout_DX;
    }
    else {
      in_DX = CONCAT11((char)uVar13,(char)puVar11);
      FUN_1000_0414();
      FUN_1000_0414();
    }
    puVar11 = puVar11 + 1;
    if ((int)(uint)DAT_2cff_2a2c < (int)puVar11) {
      puVar11 = (undefined1 *)(uint)DAT_2cff_2a2a;
      uVar13 = uVar13 + 1;
    }
    param_3 = iVar10;
    if ((int)(uint)DAT_2cff_2a2d < (int)uVar13) {
      in_stack_0000ffec = CONCAT11(DAT_2cff_2a2e,DAT_2cff_2a2d);
      FUN_2ca9_0031(1,in_stack_0000ffec,CONCAT11(DAT_2cff_2a2d,DAT_2cff_2a2c),
                    CONCAT11(DAT_2cff_2a2c,DAT_2cff_2a2b),CONCAT11(DAT_2cff_2a2b,DAT_2cff_2a2a),6);
      uVar13 = uVar13 - 1;
      in_DX = extraout_DX_00;
    }
  }
  switch(uVar14) {
  case 0:
    FUN_22ab_05b4();
    DAT_2cff_b6b4 = 0x19;
    iVar10 = FUN_26f4_2227();
    if (iVar10 == 3) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + unaff_SI * 0x1f) = 2;
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) = 0;
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bb + unaff_SI * 0x1f) = 2;
    }
    iVar10 = FUN_26f4_2227();
    if (iVar10 == 3) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + unaff_SI * 0x1f) = 2;
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) = 0;
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bb + unaff_SI * 0x1f) = 0;
    }
    if (DAT_2cff_3c7a == 0) {
      if (DAT_2cff_3c78 != 0) {
        if ((((*(uint *)((int)(undefined2 *)&DAT_2cff_96ad + unaff_SI * 0x1f) & 0xf) == 0) &&
            (iVar10 = FUN_26f4_2227(), iVar10 != 0)) && (iVar10 = FUN_26f4_2227(), iVar10 == 0)) {
          FUN_26f4_1ad4();
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + unaff_SI * 0x1f) = 3;
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bb + unaff_SI * 0x1f) = 3;
        }
        if ((*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + unaff_SI * 0x1f) == 0) &&
           (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + unaff_SI * 0x1f) == 0)) {
          if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) + DAT_2cff_3c78 * 2 < -3
             ) {
            iVar10 = -3;
          }
          else {
            iVar10 = *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) +
                     DAT_2cff_3c78 * 2;
          }
          if (iVar10 < 3) {
            if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) + DAT_2cff_3c78 * 2 <
                -3) {
              iVar10 = -3;
            }
            else {
              iVar10 = *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) +
                       DAT_2cff_3c78 * 2;
            }
          }
          else {
            iVar10 = 3;
          }
          *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) = iVar10;
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + unaff_SI * 0x1f) = 0;
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c7 + unaff_SI * 0x1f) = 0;
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bd + unaff_SI * 0x1f) = 3;
          if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) < 2) {
            if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) < -1) {
              local_4 = 0xfffe;
            }
          }
          else {
            local_4 = 2;
          }
        }
      }
    }
    else {
      DAT_2cff_3d9e = 1;
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + unaff_SI * 0x1f) = 2;
      iVar10 = FUN_26f4_1a89(9);
      *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) = -(iVar10 * 4 + 0x10);
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bb + unaff_SI * 0x1f) = 0;
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + unaff_SI * 0x1f) = DAT_2cff_3c76;
      FUN_1581_0178(1,1);
    }
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + unaff_SI * 0x1f) != 0) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c7 + unaff_SI * 0x1f) =
           *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + unaff_SI * 0x1f);
    }
    if (((DAT_2cff_3c7e != 0) &&
        ((((undefined2 *)&DAT_2cff_be2c)
          [*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + unaff_SI * 0x1f)] & 1) != 0)) &&
       (DAT_2cff_3da0 = 1, *(int *)((int)(undefined2 *)&DAT_2cff_96c7 + unaff_SI * 0x1f) != 0)) {
      iStackY_32 = 0;
      for (iStackY_2c = 0; iStackY_2c < DAT_2cff_becc; iStackY_2c = iStackY_2c + 1) {
        iStackY_32 = iStackY_32 +
                     (uint)(((undefined1 *)&DAT_2cff_96ac)
                            [((undefined2 *)&DAT_2cff_bcac)[iStackY_2c] * 0x1f] == '2');
      }
      iVar10 = FUN_26f4_1a89(8);
      if (iStackY_32 < iVar10) {
        FUN_26f4_1892(0x32,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + unaff_SI * 0x1f) +
                           *(int *)((int)(undefined2 *)&DAT_2cff_96c7 + unaff_SI * 0x1f) * 0xc + 4,
                      *(int *)((int)(undefined2 *)&DAT_2cff_96af + unaff_SI * 0x1f) + 2);
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + (DAT_2cff_bb1c + -1) * 0x1f) =
             *(int *)((int)(undefined2 *)&DAT_2cff_96c7 + unaff_SI * 0x1f) * 6;
        FUN_1581_0178(2,0x22);
      }
      else {
        iVar10 = FUN_26f4_1a38(2);
        if (iVar10 == 0) {
          FUN_1581_0178(2,8);
        }
        else {
          iVar10 = FUN_26f4_1892(2,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + unaff_SI * 0x1f) +
                                   *(int *)((int)(undefined2 *)&DAT_2cff_96c7 + unaff_SI * 0x1f) *
                                   0xc + 4,
                                 *(int *)((int)(undefined2 *)&DAT_2cff_96af + unaff_SI * 0x1f) + 2);
          if (iVar10 != 0) {
            iVar9 = FUN_26f4_2227(iVar10,*(undefined2 *)
                                          ((int)(undefined2 *)&DAT_2cff_96ad + iVar10 * 0x1f),
                                  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + iVar10 * 0x1f)
                                  ,1);
            if (iVar9 == 1) {
              *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + (DAT_2cff_bb1c + -1) * 0x1f) =
                   *(int *)((int)(undefined2 *)&DAT_2cff_96c7 + unaff_SI * 0x1f) << 3;
              *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bd + (DAT_2cff_bb1c + -1) * 0x1f) = 3;
              FUN_1581_0178(2,3);
            }
            else {
              FUN_26f4_1863(iVar10);
              FUN_26f4_1a13(2);
              FUN_1581_0178(2,8);
            }
          }
        }
      }
    }
    piVar2 = (int *)((int)(undefined2 *)&DAT_2cff_96bd + unaff_SI * 0x1f);
    *piVar2 = *piVar2 + 1;
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + unaff_SI * 0x1f) != 0) {
      piVar2 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + unaff_SI * 0x1f);
      *piVar2 = *piVar2 + -1;
    }
    FUN_26f4_23e1(0);
    FUN_2cff_2afd((char *)s_JILL_SPLASHES_INTO_THE_UNDERGROU_2cff_26ea + 10,local_4);
    uVar13 = FUN_1fae_1d0f();
    return uVar13;
  case 1:
    pcVar3 = (char *)(uVar14 * 2 + unaff_SI);
    *pcVar3 = *pcVar3 + (char)uVar14;
    return uVar14;
  case 2:
    cVar6 = *(char *)(uVar13 + 0xc);
    uVar5 = in(in_stack_00000000);
    *puVar11 = uVar5;
    bVar8 = (byte)((uint)param_1 >> 8) | puVar11[0xd];
    uVar14 = CONCAT11(bVar8,(char)param_1);
    bVar16 = true;
    if (bVar8 != 0) {
      uVar14 = uVar14 + 0xaff6 | 0x6c;
      bVar16 = ((char)((uint)in_stack_00000002 >> 8) == '\0' && cVar6 == '\0') &&
               *(char *)(unaff_BP + 0xc) == '\0';
    }
    if (bVar16) {
      lVar7 = (long)*(int *)(uVar21 + uVar13) * 0xc67;
      pcVar3 = (char *)(unaff_BP + (int)(puVar11 + 1));
      *pcVar3 = (*pcVar3 - (char)unaff_BP) - ((int)lVar7 != lVar7);
      uVar14 = uVar14 + 0xdb0f;
    }
    else {
      piVar2 = (int *)(puVar11 + 1);
      *piVar2 = *piVar2 - uVar14;
    }
    pbVar4 = (byte *)(unaff_BP * 2 + uVar13);
    *pbVar4 = *pbVar4 & (byte)(uVar14 >> 8);
                    /* WARNING: Bad instruction - Truncating control flow here */
    halt_baddata();
  case 4:
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96bb + uVar14 * in_DX) < 0) {
      iVar10 = 4;
    }
    else {
      iVar10 = 0;
    }
    uVar13 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,puVar11 + in_stack_0000ffec + iVar10,
                           *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + unaff_SI * 0x1f),
                           *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + unaff_SI * 0x1f));
    return uVar13;
  case 5:
    if ((int)local_4 < 0) {
      local_4 = -local_4;
    }
    return local_4;
  case 6:
    *(undefined2 *)(uVar14 * 2 + 7) = 0xfff8;
    iVar10 = FUN_26f4_1d73();
    if (((iVar10 == 0) && (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) != 0)) &&
       (iVar10 = FUN_26f4_1d73(), iVar10 == 0)) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) = 0;
    }
    FUN_26f4_2605();
    FUN_2cff_2afd((char *)s_JILL_SPLASHES_INTO_THE_UNDERGROU_2cff_26ea + 10,0);
    return 1;
  }
  do {
    puVar1 = (uint *)(uVar14 * 0x1f + -0x693f);
    *puVar1 = *puVar1 & (uint)puVar11;
    uVar14 = unaff_SI;
    do {
      uVar14 = uVar14 + 1;
      if ((DAT_2cff_bb1c <= (int)uVar14) || (0xbf < DAT_2cff_becc)) {
        DAT_2cff_bab0 = 0;
        DAT_2cff_bab2 = 0;
        DAT_2cff_43a2 = DAT_2cff_96ad;
        DAT_2cff_43a4 = DAT_2cff_96af;
        for (uStackY_1c = 0; (int)uStackY_1c < DAT_2cff_becc; uStackY_1c = uStackY_1c + 1) {
          iVar9 = ((undefined2 *)&DAT_2cff_bcac)[uStackY_1c];
          iVar18 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + iVar9 * 0x1f);
          iVar10 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + iVar9 * 0x1f);
          iVar19 = *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + iVar9 * 0x1f);
          iVar20 = *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + iVar9 * 0x1f);
          if (param_1 == 0) {
            puVar1 = (uint *)(iVar9 * 0x1f + -0x693f);
            *puVar1 = *puVar1 | 0xc000;
          }
          else {
            if (0 < *(int *)((int)(undefined2 *)&DAT_2cff_96c9 + iVar9 * 0x1f)) {
              piVar2 = (int *)((int)(undefined2 *)&DAT_2cff_96c9 + iVar9 * 0x1f);
              *piVar2 = *piVar2 + -1;
            }
            iVar20 = 0x2000;
            iVar19 = -0x7c04;
            iVar12 = (*(code *)((undefined2 *)&DAT_2cff_b84e)
                               [(char)((undefined1 *)&DAT_2cff_96ac)[iVar9 * 0x1f] * 2])
                               (0x2000,iVar9,2,0);
            if (iVar12 != 0) {
              puVar1 = (uint *)(iVar9 * 0x1f + -0x693f);
              *puVar1 = *puVar1 | 0xc000;
            }
          }
          if ((((undefined2 *)&DAT_2cff_be3c)[(char)((undefined1 *)&DAT_2cff_96ac)[iVar9 * 0x1f]] &
              8) != 0) {
            for (iStackY_1a = 0; iStackY_1a <= DAT_2cff_becc; iStackY_1a = iStackY_1a + 1) {
              iVar12 = ((undefined2 *)&DAT_2cff_bcac)[iStackY_1a];
              if (((iVar12 != iVar9) &&
                  (*(int *)((int)(undefined2 *)&DAT_2cff_96ad + iVar12 * 0x1f) <
                   *(int *)((int)(undefined2 *)&DAT_2cff_96ad + iVar9 * 0x1f) +
                   *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + iVar9 * 0x1f))) &&
                 ((*(int *)((int)(undefined2 *)&DAT_2cff_96ad + iVar9 * 0x1f) <
                   *(int *)((int)(undefined2 *)&DAT_2cff_96ad + iVar12 * 0x1f) +
                   *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + iVar12 * 0x1f) &&
                  ((*(int *)((int)(undefined2 *)&DAT_2cff_96af + iVar12 * 0x1f) <
                    *(int *)((int)(undefined2 *)&DAT_2cff_96af + iVar9 * 0x1f) +
                    *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + iVar9 * 0x1f) &&
                   (*(int *)((int)(undefined2 *)&DAT_2cff_96af + iVar9 * 0x1f) <
                    *(int *)((int)(undefined2 *)&DAT_2cff_96af + iVar12 * 0x1f) +
                    *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + iVar12 * 0x1f))))))) {
                (*(code *)((undefined2 *)&DAT_2cff_b84e)
                          [(char)((undefined1 *)&DAT_2cff_96ac)[iVar9 * 0x1f] * 2])
                          (0x2000,iVar9,1,iVar12);
                puVar1 = (uint *)(iVar9 * 0x1f + -0x693f);
                *puVar1 = *puVar1 | 0xc000;
                iVar20 = 0x2000;
                iVar19 = -0x7a34;
                (*(code *)((undefined2 *)&DAT_2cff_b84e)
                          [(char)((undefined1 *)&DAT_2cff_96ac)[iVar12 * 0x1f] * 2])
                          (0x2000,iVar12,1,iVar9);
                puVar1 = (uint *)(iVar12 * 0x1f + -0x693f);
                *puVar1 = *puVar1 | 0xc000;
              }
            }
          }
          if ((*(uint *)(iVar9 * 0x1f + -0x693f) & 0xc000) != 0) {
            local_4 = iVar10 + iVar20 + 0xf >> 4;
            iVar9 = iVar18 >> 4;
            for (iVar10 = iVar10 >> 4; iVar10 < (int)local_4; iVar10 = iVar10 + 1) {
              iVar20 = iVar9;
              for (; iVar9 < iVar18 + iVar19 + 0xf >> 4; iVar9 = iVar9 + 1) {
                iVar20 = 0x2cff;
                ((undefined2 *)&DAT_2cff_43a6)[iVar9 * 0x40 + iVar10] =
                     ((undefined2 *)&DAT_2cff_43a6)[iVar9 * 0x40 + iVar10] | 0xc000;
              }
              iVar9 = iVar20;
            }
          }
        }
        for (uStackY_1c = 0; (int)uStackY_1c < DAT_2cff_becc; uStackY_1c = uStackY_1c + 1) {
          iVar9 = ((undefined2 *)&DAT_2cff_bcac)[uStackY_1c];
          iVar18 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + iVar9 * 0x1f);
          bVar16 = false;
          for (iVar10 = iVar18 >> 4; iVar19 = 0x2cff,
              iVar10 < *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + iVar9 * 0x1f) + iVar18 + 0xf >>
                       4; iVar10 = iVar10 + 1) {
            for (; iVar19 < *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + iVar9 * 0x1f) +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96ad + iVar9 * 0x1f) + 0xf >> 4;
                iVar19 = iVar19 + 1) {
              if ((bVar16) ||
                 ((((undefined2 *)&DAT_2cff_43a6)[iVar19 * 0x40 + iVar10] & 0xc000) != 0)) {
                bVar16 = true;
              }
              else {
                bVar16 = false;
              }
            }
          }
          if (bVar16) {
            puVar1 = (uint *)(iVar9 * 0x1f + -0x693f);
            *puVar1 = *puVar1 | 0xc000;
          }
        }
        return uStackY_1c;
      }
      iVar10 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + uVar14 * 0x1f);
    } while ((((iVar10 + *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + uVar14 * 0x1f) < iVar10) ||
              ((int)uVar13 < *(int *)((int)(undefined2 *)&DAT_2cff_96ad + uVar14 * 0x1f))) ||
             (*(int *)((int)(undefined2 *)&DAT_2cff_96af + uVar14 * 0x1f) +
              *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + uVar14 * 0x1f) < (int)uVar21)) ||
            ((int)local_4 < *(int *)((int)(undefined2 *)&DAT_2cff_96af + uVar14 * 0x1f)));
    ((undefined2 *)&DAT_2cff_bcac)[DAT_2cff_becc] = uVar14;
    DAT_2cff_becc = DAT_2cff_becc + 1;
    puVar11 = (undefined1 *)(DAT_2cff_3a50 << 0xe ^ 0xffff);
    unaff_SI = uVar14;
  } while( true );
}


