// FUN_22ab_1531 @ 22ab:1531

void __cdecl16far FUN_22ab_1531(int param_1)

{
  undefined2 uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  int iVar6;
  int local_6;
  
  iVar6 = 0;
  local_6 = 0;
  FUN_22ab_05b4((char *)s_PRESS_F1_FOR_HELP__2cff_1367,0x2cff,4);
  FUN_26f4_1ab9();
  FUN_26f4_215f();
  FUN_22ab_0319();
  FUN_22ab_05dd();
  DAT_2cff_bb1e = '\0';
  FUN_1123_06c5(1);
  FUN_26f4_03c4();
  DAT_2cff_bcaa = 0;
  do {
    if (DAT_2cff_bb1e != '\0') {
      if (DAT_2cff_bb1e == '*') {
        uVar1 = FUN_2b9a_000b((undefined1 *)&DAT_2cff_bb1e,0x2cff);
        FUN_2ba8_0060((undefined1 *)&DAT_2cff_bb1e,0x2cff,0xbb1f,0x2cff,uVar1);
        FUN_2b9f_0008(0xb962,0x2cff,(undefined1 *)&DAT_2cff_bb1e,0x2cff);
        FUN_1581_069e((undefined1 *)&DAT_2cff_bb1e,0x2cff);
        DAT_2cff_bb1e = '\0';
      }
      else if (DAT_2cff_bb1e == '#') {
        uVar1 = FUN_2b9a_000b((undefined1 *)&DAT_2cff_bb1e,0x2cff);
        FUN_2ba8_0060((undefined1 *)&DAT_2cff_bb1e,0x2cff,0xbb1f,0x2cff,uVar1);
        FUN_2b9f_0008(0xb962,0x2cff,(undefined1 *)&DAT_2cff_bb1e,0x2cff);
        iVar2 = FUN_1581_0660();
        if (iVar2 == 0) {
          FUN_1581_069e((undefined1 *)&DAT_2cff_bb1e,0x2cff);
        }
        DAT_2cff_bb1e = '\0';
      }
      else if (DAT_2cff_bb1e == '&') {
        uVar1 = FUN_2b9a_000b((undefined1 *)&DAT_2cff_bb1e,0x2cff);
        FUN_2ba8_0060((undefined1 *)&DAT_2cff_bb1e,0x2cff,0xbb1f,0x2cff,uVar1);
        DAT_2cff_3dca = 2;
        FUN_1415_05fd((undefined1 *)&DAT_2cff_bb1e,0x2cff);
        FUN_2b9f_0008(0xb962,0x2cff,(undefined1 *)&DAT_2cff_bb1e,0x2cff);
        iVar2 = FUN_1581_0660();
        if (iVar2 == 0) {
          FUN_1581_069e((undefined1 *)&DAT_2cff_bb1e,0x2cff);
        }
        DAT_2cff_bb1e = '\0';
      }
      else if (DAT_2cff_bb1e == '!') {
        FUN_22ab_124b(0);
        iVar3 = FUN_26f4_1a89(3);
        iVar2 = DAT_2cff_83cc;
        FUN_22ab_0aa5(0x137a,0x2cff);
        DAT_2cff_83ce = iVar2 >> 0xf;
        DAT_2cff_83cc = iVar2;
        FUN_2b94_000d(0x137f,0x2cff);
        iVar2 = iVar3;
        while (0 < iVar2) {
          FUN_26f4_1a13(3);
          iVar2 = iVar2 + -1;
        }
        DAT_2cff_bb1e = '\0';
        FUN_26f4_04c7(0);
        iVar4 = FUN_26f4_037d(DAT_2cff_83a6);
        iVar2 = *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + iVar4 * 0x1f);
        if (iVar2 == 0) {
          FUN_26f4_1863(iVar4);
        }
        else if (iVar2 == 1) {
          if (iVar3 < 1) {
            FUN_22ab_05b4((char *)s_YOU_LEFT_WITHOUT_FINDING_A_GEM__2cff_1384,0x2cff,4);
            FUN_26f4_1ad4(0,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + iVar4 * 0x1f),
                          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + iVar4 * 0x1f));
            DAT_2cff_83a6 = 0;
          }
          else {
            FUN_26f4_1863(iVar4);
          }
        }
        else {
          FUN_26f4_1863(iVar4);
        }
        FUN_22ab_13a1();
      }
      else {
        DAT_2cff_beca = DAT_2cff_83a6;
        FUN_22ab_124b(DAT_2cff_83a6);
        FUN_22ab_0c9a(0x13a4,0x2cff);
        iVar2 = DAT_2cff_83cc;
        FUN_22ab_0aa5((undefined1 *)&DAT_2cff_bb1e,0x2cff);
        DAT_2cff_83ce = iVar2 >> 0xf;
        DAT_2cff_bb1e = '\0';
        DAT_2cff_83a6 = DAT_2cff_beca;
        DAT_2cff_83cc = iVar2;
        FUN_26f4_04c7(0);
        FUN_22ab_13a1();
      }
    }
    iVar2 = DAT_2cff_3e28;
    FUN_1581_063e();
    DAT_2cff_be3a = DAT_2cff_be3a + 1;
    FUN_1415_026f(1);
    DAT_2cff_3c86 = FUN_2ce7_000d(DAT_2cff_3c86);
    if (DAT_2cff_3c86 != 0) {
      if ((iVar6 == 0x2f) && (local_6 == 2)) {
        iVar6 = 0;
        if (DAT_2cff_3c86 == 0x30) {
          if (DAT_2cff_3dc2 != 0) {
            FUN_1415_06eb();
          }
        }
        else if (DAT_2cff_3c86 == 0x43) {
          FUN_1415_05fd((char *)s_temp_mac_2cff_13b2,0x2cff);
        }
        else if (DAT_2cff_3c86 == 0x52) {
          FUN_1415_06a4((char *)s_temp_mac_2cff_13a9,0x2cff);
        }
        DAT_2cff_3c86 = 0;
      }
      if (DAT_2cff_3c86 == iVar6) {
        local_6 = local_6 + 1;
      }
      else {
        local_6 = 1;
        iVar6 = DAT_2cff_3c86;
      }
      if ((iVar6 == 0x58) && (local_6 == 3)) {
        iVar6 = 0;
        DAT_2cff_83a8 = 8;
        iVar3 = FUN_26f4_1a89(10);
        if (iVar3 == 0) {
          FUN_26f4_1a13(10);
        }
        iVar3 = FUN_26f4_1a89(1);
        if (iVar3 == 0) {
          FUN_26f4_1a13(1);
        }
        DAT_2cff_be38 = DAT_2cff_be38 | 0xc000;
      }
      else if ((iVar6 == 0x5a) && (local_6 == 3)) {
        iVar6 = 0;
        DAT_2cff_120a = (uint)(DAT_2cff_120a == 0);
        DAT_2cff_be38 = DAT_2cff_be38 | 0xc000;
      }
      else if ((iVar6 == 0x57) && (local_6 == 3)) {
        FUN_1415_059e();
        FUN_150f_0008(DAT_2cff_3c86 + -0x30);
        DAT_2cff_120c = 1;
        iVar6 = 0;
      }
    }
    if (DAT_2cff_3c86 == 0x4e) {
      DAT_2cff_3e26 = (uint)(DAT_2cff_3e26 == 0);
      DAT_2cff_be38 = DAT_2cff_be38 | 0xc000;
    }
    else if (DAT_2cff_3c86 == 0x50) {
      while( true ) {
        FUN_1415_026f(0);
        FUN_1581_063e();
        if (((DAT_2cff_3c86 != 0) || (DAT_2cff_3c7a != 0)) || (DAT_2cff_3c7e != 0)) break;
        if ((DAT_2cff_3c76 != 0) || (DAT_2cff_3c78 != 0)) break;
      }
    }
    else if (DAT_2cff_3c86 == 0x54) {
      DAT_2cff_1210 = (uint)(DAT_2cff_1210 == 0);
      DAT_2cff_be38 = DAT_2cff_be38 | 0xc000;
    }
    if ((param_1 != 0) && (iVar3 = FUN_26f4_00f2(0x43), iVar3 == 0)) {
      FUN_26f4_1892(0x43,DAT_2cff_96ad,DAT_2cff_96af);
    }
    FUN_26f4_11d7();
    FUN_26f4_12b9(1);
    FUN_26f4_259b();
    FUN_26f4_0d52(DAT_2cff_3a44);
    FUN_26f4_2506();
    iVar3 = FUN_2ce7_000d(DAT_2cff_3c86);
    iVar4 = 5;
    piVar5 = (int *)0x19df;
    do {
      if (iVar3 == *piVar5) {
                    /* WARNING: Could not recover jumptable at 0x0002448b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (*(code *)piVar5[5])();
        return;
      }
      piVar5 = piVar5 + 1;
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
    if ((param_1 != 0) && (DAT_2cff_3dc8 == 0)) {
      DAT_2cff_bcaa = 1;
    }
    do {
    } while (DAT_2cff_3e28 - iVar2 < (int)(DAT_2cff_1210 + 1));
    if (DAT_2cff_bcaa != 0) {
      DAT_2cff_3c86 = 0;
      if (DAT_2cff_bcaa == 2) {
        FUN_22ab_266f(200);
      }
      FUN_1123_06c5(0);
      if (param_1 == 0) {
        FUN_22ab_1c54(1);
      }
      return;
    }
  } while( true );
}


