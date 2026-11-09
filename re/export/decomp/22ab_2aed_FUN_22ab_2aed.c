// FUN_22ab_2aed @ 22ab:2aed

void __cdecl16far FUN_22ab_2aed(void)

{
  bool bVar1;
  int iVar2;
  uint uVar3;
  
  bVar1 = false;
  FUN_22ab_0aa5(0x1777,0x2cff);
  do {
    FUN_1123_06c5(1);
    FUN_26f4_215f();
    FUN_22ab_14a9();
    FUN_22ab_1c54(0);
    FUN_1353_0b4a((undefined2 *)&DAT_2cff_b69e,0x2cff);
    FUN_1353_0b64(DAT_2cff_b982,DAT_2cff_b984,7,8);
    FUN_1353_0b4a(DAT_2cff_b982,DAT_2cff_b984);
    if ((DAT_2cff_17ea == 0) || (DAT_2cff_3a4d != '\x04')) {
      FUN_1353_074e(DAT_2cff_b982,DAT_2cff_b984,0,0x1c,2,DAT_2cff_2519,DAT_2cff_251b);
      FUN_1353_074e(DAT_2cff_b982,DAT_2cff_b984,0,0x24,2,DAT_2cff_251d,DAT_2cff_251f);
    }
    else {
      for (uVar3 = 0; (int)uVar3 < 0x10; uVar3 = uVar3 + 1) {
        FUN_1123_00eb(DAT_2cff_b982,DAT_2cff_b984,DAT_2cff_17ea * 0x100 + uVar3 + 0x4000,
                      (uVar3 & 3) << 4,((int)uVar3 >> 2) << 4);
      }
    }
    FUN_1123_0716();
    FUN_1123_06c5(0);
    FUN_22ab_2814((char *)s_7PICK_A_CHOICE__2PLAY_2RESTORE_5_2cff_14a8,0x2cff,0x150d,0x2cff,1,9,1,
                  0x18,9,8);
    FUN_1123_06c5(0);
    if (DAT_2cff_3c86 == 5) {
      FUN_1123_06c5(1);
      FUN_22ab_2523();
      FUN_22ab_14a9();
      FUN_1123_0716();
      FUN_1123_06c5(0);
      FUN_25eb_09c0();
    }
    else if ((DAT_2cff_3c86 == 0x1b) || (DAT_2cff_3c86 == 0x51)) {
      bVar1 = true;
    }
    else if (DAT_2cff_3c86 == 0x50) {
      DAT_2cff_be3a = 0;
      FUN_1123_06c5(1);
      FUN_22ab_001f();
      FUN_22ab_2523();
      FUN_22ab_0319();
      FUN_22ab_0aa5(5999,0x2cff);
      DAT_2cff_83a6 = DAT_2cff_17ec;
      FUN_26f4_04c7(0);
      FUN_22ab_14a9();
      FUN_1123_0716();
      FUN_1123_06c5(0);
      FUN_22ab_0006();
      FUN_22ab_1531(0);
      FUN_1581_069e(0x1781,0x2cff);
      FUN_22ab_0aa5(0x1777,0x2cff);
    }
    else if (DAT_2cff_3c86 == 0x10) {
      FUN_1123_06c5(1);
      FUN_22ab_2523();
      FUN_22ab_14a9();
      FUN_22ab_0319();
      FUN_1123_0716();
      FUN_1123_06c5(0);
      FUN_26f4_03c4();
      DAT_2cff_bafc = 0;
      FUN_22ab_1531(0);
    }
    else if (DAT_2cff_3c86 == 0x52) {
      iVar2 = FUN_22ab_22b2();
      if (iVar2 != 0) {
        FUN_1123_06c5(1);
        FUN_22ab_001f();
        FUN_22ab_2523();
        FUN_22ab_0319();
        FUN_26f4_215f();
        FUN_22ab_14a9();
        FUN_26f4_03c4();
        FUN_1123_0716();
        FUN_1123_06c5(0);
        FUN_22ab_0006();
        FUN_22ab_1531(0);
        FUN_1581_069e(0x1781,0x2cff);
        FUN_22ab_0aa5(0x1777,0x2cff);
      }
    }
    else if (DAT_2cff_3c86 == 0x53) {
      FUN_22ab_266f(0);
    }
    else if (DAT_2cff_3c86 == 0x49) {
      FUN_22ab_0f0a(1);
    }
    else if (DAT_2cff_3c86 == 0x4f) {
      FUN_22ab_266f(8);
    }
    else if (DAT_2cff_3c86 == 0x43) {
      FUN_22ab_266f(0xc);
    }
    else if (DAT_2cff_3c86 == 0x44) {
      FUN_1123_06c5(1);
      DAT_2cff_be3a = 0;
      FUN_22ab_001f();
      FUN_22ab_2523();
      FUN_22ab_0319();
      DAT_2cff_b996 = 0;
      DAT_2cff_3dca = 0;
      do {
        if (DAT_2cff_b996 != 0) {
          FUN_22ab_001f();
          FUN_1123_06c5(1);
        }
        if (((undefined1 *)&DAT_2cff_1818)[DAT_2cff_b996] == '\0') {
          DAT_2cff_b996 = 0;
        }
        FUN_22ab_0aa5(((undefined2 *)&DAT_2cff_17f0)[DAT_2cff_b996 * 2],
                      ((undefined2 *)&DAT_2cff_17f2)[DAT_2cff_b996 * 2]);
        DAT_2cff_83a6 = (int)(char)((undefined1 *)&DAT_2cff_1818)[DAT_2cff_b996];
        FUN_26f4_04c7(0);
        FUN_22ab_14a9();
        FUN_1123_0716();
        FUN_1123_06c5(0);
        FUN_22ab_0006();
        FUN_1415_05fd(((undefined2 *)&DAT_2cff_1822)[DAT_2cff_b996 * 2],
                      ((undefined2 *)&DAT_2cff_1824)[DAT_2cff_b996 * 2]);
        if (DAT_2cff_3dc8 == 0) {
          DAT_2cff_3dcc = 1;
        }
        else {
          FUN_22ab_1531(1);
          FUN_1415_05b2();
          DAT_2cff_b996 = DAT_2cff_b996 + 1;
        }
      } while (DAT_2cff_3dcc == 0);
      FUN_1581_069e(0x1781,0x2cff);
      FUN_22ab_0aa5(0x1777,0x2cff);
    }
    else if (DAT_2cff_3c86 == 0x4e) {
      FUN_22ab_266f(99);
    }
  } while (!bVar1);
  return;
}


