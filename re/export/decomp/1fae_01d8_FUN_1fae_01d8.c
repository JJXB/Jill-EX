// FUN_1fae_01d8 @ 1fae:01d8

ulong FUN_1fae_01d8(undefined2 param_1,int *param_2,undefined2 param_3,undefined1 *param_4)

{
  uint *puVar1;
  int *piVar2;
  long lVar3;
  int *piVar4;
  byte bVar5;
  undefined1 uVar6;
  uint uVar7;
  undefined2 uVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  uint uVar12;
  undefined2 uVar13;
  uint in_BX;
  uint uVar14;
  int iVar15;
  undefined2 unaff_SS;
  undefined4 uVar16;
  ulong uVar17;
  undefined1 uStack01c1;
  byte bStack_32;
  byte bStack_31;
  int local_2e [4];
  undefined2 uStack_26;
  undefined2 uStack_24;
  undefined1 local_22;
  byte bStack_21;
  byte bStack_20;
  byte bStack_1f;
  char cStack_1e;
  byte bStack_1d;
  uint uStack_1c;
  uint local_1a;
  undefined4 uStack_18;
  undefined1 *puStack_14;
  byte bStack_10;
  byte bStack_f;
  undefined2 uStack_e;
  undefined2 uStack_c;
  int iStack_a;
  uint uStack_8;
  undefined2 uStack_6;
  uint local_4;
  
  piVar2 = param_2;
  iVar9 = DAT_2cff_bb3e;
  piVar4 = (int *)param_2;
  iVar15 = DAT_2cff_bb3e * 0x100;
  local_2e[0] = 0;
  FUN_1000_05d8(0x11d0,0x2cff,local_2e + 2,unaff_SS);
  FUN_1000_05d8(0x11d7,0x2cff,&local_22,unaff_SS);
  iVar10 = 0x15;
  uVar16 = FUN_1000_05d8(0x11de,0x2cff,&local_1a,unaff_SS);
  iVar11 = (int)((ulong)uVar16 >> 0x10);
  local_4 = 0;
  if (param_2._2_2_ != 0) {
    puVar1 = (uint *)(&uStack01c1 + iVar15);
    uVar12 = *puVar1;
    *puVar1 = *puVar1 << 1 | (uint)((int)uVar12 < 0);
    uVar7 = (uint)uVar16 - in_BX;
    if ((uint)uVar16 < in_BX) {
      iVar11 = iVar11 + -0x1000;
    }
    return CONCAT22(iVar11 + (uint)(byte)(((char)iVar10 + ((int)uVar12 < 0)) * '\x10') * -0x100 +
                    (uVar7 >> 4),uVar7) & 0xffff000f;
  }
  lVar3 = ((ulong)piVar2 & 0xffff) * 0x1f;
  uVar7 = (uint)((ulong)lVar3 >> 0x10);
  uVar12 = *(uint *)((int)(undefined2 *)&DAT_2cff_96b9 + (int)lVar3);
  if (5 < uVar12) {
    uVar17 = FUN_1000_0272();
    return uVar17;
  }
  iVar11 = uVar12 * 2;
  switch(uVar12) {
  case 0:
    FUN_1069_00de((int)&param_2 + 2);
    FUN_1069_00de((int)&param_2 + 2);
    FUN_1069_00de((int)&param_2 + 2);
    FUN_1069_00de((int)&param_2 + 2);
    FUN_1069_00de((int)&param_2 + 2);
    if (DAT_2cff_3a4d == '\0') {
      uStack_24 = local_2e[2];
      uStack_e = 3;
      uStack_c = 0;
    }
    else if (DAT_2cff_3a4d == '\x01') {
      uStack_24 = local_2e[2];
      uStack_e = 3;
      uStack_c = 4;
    }
    else if (DAT_2cff_3a4d == '\x02') {
      uStack_24 = local_2e[3];
      uStack_e = 0xf;
      uStack_c = 8;
    }
    else if (DAT_2cff_3a4d == '\x03') {
      uStack_24 = local_2e[3];
      uStack_e = 0xf;
      uStack_c = 0xc;
    }
    else {
      uStack_24 = uStack_26;
      uStack_e = 0xff;
      uStack_c = 0x10;
    }
    if ((local_1a & 1) == 0) {
      if (bStack_31 == 8) {
        FUN_1069_004d();
      }
      else {
        for (bStack_1d = 0; uVar7 = (uint)bStack_1d, (int)uVar7 < 1 << (bStack_31 & 0x1f);
            bStack_1d = bStack_1d + 1) {
          FUN_1069_00de((int)&param_2 + 2);
          bVar5 = FUN_1000_0595();
          *(byte *)(bStack_1d + 0x2e5c) = bVar5 & (byte)uStack_e;
        }
      }
    }
    else {
      for (bStack_1d = 0; uVar7 = (uint)bStack_1d, (int)uVar7 < 1 << (bStack_31 & 0x1f);
          bStack_1d = bStack_1d + 1) {
        *(byte *)(bStack_1d + 0x2e5c) = bStack_1d;
      }
    }
    iStack_a = FUN_2ae9_000a(uStack_24);
    uStack_8 = uVar7;
    if (iStack_a == 0 && uVar7 == 0) {
      FUN_22ab_3177(9);
    }
    *(undefined2 *)((int)(int *)param_2 * 2 + 0x30ae) = uStack_24;
    ((undefined2 *)&DAT_2cff_3130)[(int)(int *)param_2 * 2] = uStack_8;
    ((undefined2 *)&DAT_2cff_312e)[(int)(int *)param_2 * 2] = iStack_a;
    *(uint *)((int)(int *)param_2 * 2 + 0x302e) = local_1a;
    uStack_6 = 0;
    uVar7 = (uint)bStack_32 << 2;
    uVar12 = uStack_8;
    local_4 = uVar7;
    for (bStack_1d = 0; bStack_1d < bStack_32; bStack_1d = bStack_1d + 1) {
      FUN_1069_00de((int)&param_2 + 2);
      FUN_1069_00de((int)&param_2 + 2);
      FUN_1069_00de((int)&param_2 + 2);
      uStack_18 = param_4;
      if (cStack_1e == '\0') {
        FUN_2ba2_0001((undefined1 *)param_4,(int)((ulong)param_4 >> 0x10),param_2._2_2_,param_3,
                      (uint)bStack_21 * (uint)bStack_1f);
        uVar12 = 0;
        param_2 = (int *)CONCAT22(param_2._2_2_ + (uint)bStack_21 * (uint)bStack_1f,(int *)param_2);
      }
      else if (cStack_1e == '\x01') {
        FUN_1069_0228();
        puStack_14 = uStack_18;
        for (iVar9 = 0; iVar9 < (int)(uint)bStack_1f; iVar9 = iVar9 + 1) {
          for (iVar10 = 0; iVar10 < (int)(uint)bStack_21; iVar10 = iVar10 + 1) {
            uVar6 = FUN_1069_0233((int)&param_2 + 2);
            *puStack_14 = uVar6;
            puStack_14 = (undefined1 *)CONCAT22(puStack_14._2_2_,(undefined1 *)puStack_14 + 1);
          }
        }
      }
      if ((((bStack_31 == 8) && (bStack_21 == 0x40)) && (bStack_1f == 0xc)) &&
         (DAT_2cff_3a4d == '\x04')) {
        FUN_2ba8_0060(0x194,0x2cff,(undefined1 *)uStack_18,uStack_18._2_2_,0x300);
        FUN_1123_0791();
      }
      if ((DAT_2cff_3a4d == '\0') || (DAT_2cff_3a4d == '\x01')) {
        uVar12 = (bStack_21 + 3) % 4;
        bStack_20 = (byte)((bStack_21 + 3) / 4);
        FUN_1069_0106(iStack_a,uStack_8,&uStack_6);
        FUN_1069_0106(iStack_a,uStack_8,&uStack_6);
        FUN_1069_0106(iStack_a,uStack_8,&uStack_6);
        bStack_f = 0;
        for (uVar14 = 0; uVar7 = (uint)bStack_1f, (int)uVar14 < (int)uVar7; uVar14 = uVar14 + 1) {
          for (uVar7 = 0; (int)uVar7 < (int)(uint)bStack_21; uVar7 = uVar7 + 1) {
            uVar12 = (uint)((ulong)bStack_21 * (ulong)uVar14 >> 0x10);
            bStack_10 = *(byte *)((byte)((undefined1 *)uStack_18)
                                        [(int)((ulong)bStack_21 * (ulong)uVar14) + uVar7] + 0x2e5c);
            bStack_f = bStack_f | bStack_10 << (((byte)uVar7 & 3) * -2 + 6 & 0x1f);
            if (((uVar7 & 3) == 3) || (bStack_21 - 1 == uVar7)) {
              FUN_1069_0106(iStack_a,uStack_8,&local_4);
              bStack_f = 0;
            }
          }
        }
      }
      else if ((DAT_2cff_3a4d == '\x02') || (DAT_2cff_3a4d == '\x03')) {
        bStack_20 = bStack_21;
        FUN_1069_0106(iStack_a,uStack_8,&uStack_6);
        FUN_1069_0106(iStack_a,uStack_8,&uStack_6);
        uVar7 = FUN_1069_0106(iStack_a,uStack_8,&uStack_6);
        if ((local_1a & 4) == 0) {
          for (iVar9 = 0; uVar7 = (uint)bStack_1f, iVar9 < (int)uVar7; iVar9 = iVar9 + 1) {
            for (iVar10 = 0; iVar10 < (int)(uint)bStack_21; iVar10 = iVar10 + 2) {
              uVar8 = (undefined2)((ulong)uStack_18 >> 0x10);
              bStack_f = *(byte *)((byte)((undefined1 *)uStack_18)[(uint)bStack_21 * iVar9 + iVar10]
                                  + 0x2e5c) |
                         *(char *)((byte)((undefined1 *)uStack_18)
                                         [(uint)bStack_21 * iVar9 + iVar10 + 1] + 0x2e5c) << 4;
              uVar12 = (uint)bStack_f;
              FUN_1069_0106(iStack_a,uStack_8,&local_4);
            }
          }
        }
        else {
          for (uStack_1c = 8; 0 < (int)uStack_1c; uStack_1c = (int)uStack_1c >> 1) {
            for (iVar9 = 0; uVar7 = (uint)bStack_1f, iVar9 < (int)uVar7; iVar9 = iVar9 + 1) {
              bStack_f = 0;
              for (uVar7 = 0; (int)uVar7 < (int)(uint)bStack_21; uVar7 = uVar7 + 1) {
                bStack_10 = *(byte *)((byte)((undefined1 *)uStack_18)
                                            [(uint)bStack_21 * iVar9 + uVar7] + 0x2e5c);
                uVar12 = uVar7 & 0xff07;
                bStack_f = bStack_f | ((uStack_1c & bStack_10) != 0) << (7U - (char)uVar12 & 0x1f);
                if (((uVar7 & 7) == 7) || (bStack_21 - 1 == uVar7)) {
                  FUN_1069_0106(iStack_a,uStack_8,&local_4);
                  bStack_f = 0;
                }
              }
            }
          }
        }
      }
      else {
        bStack_20 = bStack_21;
        FUN_1069_0106(iStack_a,uStack_8,&uStack_6);
        FUN_1069_0106(iStack_a,uStack_8,&uStack_6);
        uVar7 = FUN_1069_0106(iStack_a,uStack_8,&uStack_6);
        if ((local_1a & 4) == 0) {
          for (uVar14 = 0; uVar7 = (uint)bStack_1f, (int)uVar14 < (int)uVar7; uVar14 = uVar14 + 1) {
            for (iVar9 = 0; iVar9 < (int)(uint)bStack_21; iVar9 = iVar9 + 1) {
              uVar12 = (uint)((ulong)bStack_21 * (ulong)uVar14 >> 0x10);
              bStack_f = *(byte *)((byte)((undefined1 *)uStack_18)
                                         [(int)((ulong)bStack_21 * (ulong)uVar14) + iVar9] + 0x2e5c)
              ;
              FUN_1069_0106(iStack_a,uStack_8,&local_4);
            }
          }
        }
        else {
          for (uStack_1c = 3; -1 < (int)uStack_1c; uStack_1c = uStack_1c - 1) {
            for (uVar14 = 0; uVar7 = (uint)bStack_1f, (int)uVar14 < (int)uVar7; uVar14 = uVar14 + 1)
            {
              for (iVar9 = 0; iVar9 < (int)(uint)bStack_21; iVar9 = iVar9 + 4) {
                uVar12 = (uint)((ulong)bStack_21 * (ulong)uVar14 >> 0x10);
                bStack_f = *(byte *)((byte)((undefined1 *)uStack_18)
                                           [(int)((ulong)bStack_21 * (ulong)uVar14) +
                                            iVar9 + uStack_1c] + 0x2e5c);
                FUN_1069_0106(iStack_a,uStack_8,&local_4);
              }
            }
          }
        }
      }
    }
    return CONCAT22(uVar12,CONCAT11((char)(uVar7 >> 8),bStack_1d));
  case 1:
    *(undefined2 *)(iVar11 + 0xd) = 0;
    FUN_26f4_27dd(piVar4,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + (int)piVar4 * 0x1f) +
                         *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + (int)piVar4 * 0x1f),
                  *(int *)((int)(undefined2 *)&DAT_2cff_96af + (int)piVar4 * 0x1f) +
                  *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + (int)piVar4 * 0x1f));
    FUN_26f4_1d73(piVar4,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + (int)piVar4 * 0x1f),
                  local_2e[*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + (int)piVar4 * 0x1f) + 2] +
                  *(int *)((int)(undefined2 *)&DAT_2cff_96af + (int)piVar4 * 0x1f) +
                  *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + (int)piVar4 * 0x1f));
    lVar3 = ((ulong)piVar2 & 0xffff) * 0x1f;
    uVar8 = (undefined2)((ulong)lVar3 >> 0x10);
    iVar9 = FUN_26f4_1d73(piVar4,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + (int)piVar4 * 0x1f) +
                                 *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + (int)lVar3),
                          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + (int)piVar4 * 0x1f));
    if (iVar9 == 0) {
      FUN_26f4_1863(piVar4);
    }
    else {
      lVar3 = ((ulong)piVar2 & 0xffff) * 0x1f;
      uVar8 = (undefined2)((ulong)lVar3 >> 0x10);
      iVar9 = (int)lVar3;
      piVar2 = (int *)((int)(undefined2 *)&DAT_2cff_96bb + iVar9);
      *piVar2 = *piVar2 + 1;
      if (0x4f < *(int *)((int)(undefined2 *)&DAT_2cff_96bb + iVar9)) {
        FUN_26f4_1863(piVar4);
      }
    }
    return CONCAT22(uVar8,1);
  case 3:
    iVar10 = FUN_26f4_1d73(0,DAT_2cff_96ad,uVar12);
    if (iVar10 != 0) {
      FUN_26f4_1ad4(piVar4,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + (int)piVar4 * 0x1f),
                    *(int *)((int)(undefined2 *)&DAT_2cff_96af + (int)piVar4 * 0x1f) + -0x10);
      ((undefined2 *)&DAT_2cff_43a6)[local_4 * 0x40 + iVar9 * 0x100] = 0xc08a;
    }
    lVar3 = ((ulong)piVar2 & 0xffff) * 0x1f;
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bb + (int)lVar3) = DAT_2cff_3c78;
    return CONCAT22((int)((ulong)lVar3 >> 0x10),1);
  case 4:
                    /* WARNING: Could not recover jumptable at 0x0001c00b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    uVar17 = (*(code *)*(undefined2 *)(iVar11 + 0x1bff))();
    return uVar17;
  case 5:
    FUN_2ba2_0001(param_3,(undefined1 *)param_4,*param_2,((int *)param_2)[1]);
    *param_2 = *param_2 + param_4._2_2_;
    return CONCAT22(uVar7,param_4._2_2_);
  }
  if (iVar10 == 1) {
    uVar13 = (undefined2)((ulong)uVar12 * 0x1f >> 0x10);
    uVar8 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                          *(int *)((int)&uStack_18 +
                                  *(int *)((int)(undefined2 *)&DAT_2cff_96bd +
                                          (int)((ulong)uVar12 * 0x1f)) * 2) + iVar15,
                          local_2e[*(int *)(iVar11 + 0x11)] +
                          *(int *)((int)(undefined2 *)&DAT_2cff_96ad + uVar12 * 0x1f));
    return CONCAT22(uVar13,uVar8);
  }
  uVar8 = in(0x5d);
  return CONCAT22(uVar7,uVar8);
}


