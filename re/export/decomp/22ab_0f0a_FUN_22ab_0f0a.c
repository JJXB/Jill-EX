// FUN_22ab_0f0a @ 22ab:0f0a

void __cdecl16far FUN_22ab_0f0a(undefined2 param_1)

{
  int iVar1;
  int iVar2;
  undefined1 local_b0 [24];
  undefined1 local_98 [16];
  undefined1 local_88 [6];
  int local_82;
  int local_58;
  int local_56;
  int local_54;
  undefined1 local_52 [80];
  
  DAT_2cff_3d90 = 1;
  DAT_2cff_3d92 = 1;
  FUN_1581_03ef(param_1);
  if (DAT_2cff_3f0a != 0 || DAT_2cff_3f0c != 0) {
    FUN_1123_06c5(1);
    FUN_1353_000d(local_b0);
    FUN_1353_023b(local_b0);
    FUN_1353_0b64(local_88);
    FUN_1353_0b4a(local_88);
    FUN_22ab_0dc6(0,local_52);
    FUN_1353_0b64(local_98);
    FUN_1353_0a32(local_b0);
    FUN_1353_0b64(local_88);
    local_54 = local_82 / 6;
    local_58 = FUN_22ab_0d99();
    if (local_54 < local_58) {
      iVar2 = 0;
      local_56 = 0;
      for (iVar1 = 1; iVar1 <= local_54; iVar1 = iVar1 + 1) {
        FUN_22ab_0e86(local_88);
        local_56 = local_56 + 6;
      }
      FUN_1123_0716();
      FUN_1123_06c5(0);
      DAT_2cff_3d9e = 1;
      do {
        do {
          FUN_1415_026f(1);
        } while (DAT_2cff_3c76 != 0);
      } while ((DAT_2cff_3c78 != 0) || (DAT_2cff_3c86 != 0));
      do {
        FUN_1415_026f(0);
        DAT_2cff_3c76 =
             DAT_2cff_3c76 + ((uint)(DAT_2cff_3c86 == 0xd1) - (uint)(DAT_2cff_3c86 == 0xc9));
        if ((SCARRY2(DAT_2cff_3c76,DAT_2cff_3c78) == DAT_2cff_3c76 + DAT_2cff_3c78 < 0) ||
           (iVar2 < 1)) {
          if ((DAT_2cff_3c76 + DAT_2cff_3c78 != 0 &&
               SCARRY2(DAT_2cff_3c76,DAT_2cff_3c78) == DAT_2cff_3c76 + DAT_2cff_3c78 < 0) &&
             (iVar2 + local_54 < local_58)) {
            iVar2 = iVar2 + 1;
            FUN_11f4_0e4c(local_88);
            FUN_22ab_0e86(local_88);
          }
        }
        else {
          iVar2 = iVar2 + -1;
          FUN_11f4_0e4c(local_88);
          FUN_22ab_0e86(local_88);
        }
      } while (((DAT_2cff_3c86 != 0xd) && (DAT_2cff_3c86 != 0x1b)) && (DAT_2cff_3c7a == 0));
      FUN_1123_06c5(1);
    }
    else {
      local_56 = (local_82 + (local_58 + -1) * -6) / 2;
      for (iVar1 = 1; iVar1 < local_58; iVar1 = iVar1 + 1) {
        FUN_22ab_0e86(local_88);
        local_56 = local_56 + 6;
      }
      FUN_1123_0716();
      FUN_1353_0b64(local_88);
      do {
        do {
          FUN_1415_026f(1);
        } while (DAT_2cff_3c76 != 0);
      } while (((DAT_2cff_3c78 != 0) || (DAT_2cff_3c86 != 0)) || (DAT_2cff_3c7a != 0));
      do {
        FUN_1415_026f(1);
        if ((DAT_2cff_3c86 == 0x20) || (DAT_2cff_3c86 == 0xd)) break;
      } while (DAT_2cff_3c7a == 0);
    }
    FUN_22ab_14f6();
    FUN_2c09_0008(DAT_2cff_3f0a,DAT_2cff_3f0c);
    DAT_2cff_3c86 = 0;
  }
  return;
}


