// FUN_22ab_2814 @ 22ab:2814

int __cdecl16far
FUN_22ab_2814(undefined4 param_1,undefined4 param_2,undefined2 param_3,int param_4,int param_5)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  undefined2 uVar5;
  undefined1 local_b6 [40];
  undefined1 local_8e [48];
  int local_5e;
  int local_5c;
  int local_5a;
  uint local_58;
  int local_56;
  int local_54;
  undefined1 local_52 [80];
  
  local_5e = 0;
  uVar4 = 0;
  local_56 = 0;
  local_54 = 1;
  DAT_2cff_3f0c = (undefined2)((ulong)param_1 >> 0x10);
  DAT_2cff_3f0a = (undefined2)param_1;
  DAT_2cff_439a = FUN_2b9a_000b(DAT_2cff_3f0a,DAT_2cff_3f0c);
  FUN_22ab_0d99(0,0,2);
  FUN_1353_000d(local_b6);
  FUN_1353_023b(local_b6);
  FUN_1123_06c5(1);
  local_5c = DAT_2cff_3e28;
  iVar1 = FUN_22ab_0d99();
  while (iVar1 = iVar1 + -1, -1 < iVar1) {
    FUN_22ab_0dc6(iVar1,local_52);
    FUN_1353_0b64(local_8e);
    FUN_1353_074e(local_8e);
  }
  FUN_1123_0716();
  FUN_1123_06c5(0);
  iVar1 = 0;
  while( true ) {
    FUN_1581_063e();
    if ((local_54 != iVar1) || (DAT_2cff_3e28 != local_5e)) {
      uVar4 = uVar4 + 1;
      if (0xb < (int)uVar4) {
        uVar4 = 0;
      }
      if (((uVar4 & 1) != 0) || (local_54 != iVar1)) {
        FUN_1123_00eb(local_8e);
        FUN_1123_00eb(local_8e);
      }
      local_5e = DAT_2cff_3e28;
      local_54 = iVar1;
    }
    FUN_1415_026f(0);
    DAT_2cff_3c86 = FUN_2ce7_000d(DAT_2cff_3c86);
    if ((DAT_2cff_3c76 + DAT_2cff_3c78 != 0) &&
       (uVar3 = DAT_2cff_3e28 - local_56 >> 0xf,
       1 < (int)((DAT_2cff_3e28 - local_56 ^ uVar3) - uVar3))) {
      local_56 = DAT_2cff_3e28;
      iVar1 = iVar1 + DAT_2cff_3c76 + DAT_2cff_3c78;
      if (iVar1 < 0) {
        iVar2 = 0;
      }
      else {
        iVar2 = iVar1;
      }
      if (param_4 + -1 < iVar2) {
        iVar1 = param_4 + -1;
      }
      else if (iVar1 < 0) {
        iVar1 = 0;
      }
      local_5c = DAT_2cff_3e28;
    }
    if ((300 < DAT_2cff_3e28 - local_5c) && (param_5 != 0)) break;
    local_5a = 0;
    if (DAT_2cff_3c86 == 0x1b) {
      DAT_2cff_3c86 = 0x51;
    }
    iVar2 = (int)param_2;
    uVar5 = (undefined2)((ulong)param_2 >> 0x10);
    if (((DAT_2cff_3c86 == 0xd) || (DAT_2cff_3c86 == 0x20)) || (DAT_2cff_3c7a != 0)) {
      DAT_2cff_3c86 = (int)*(char *)(iVar2 + iVar1);
      local_5a = 1;
    }
    else {
      local_58 = 0;
      while (uVar3 = FUN_2b9a_000b(iVar2,uVar5), local_58 < uVar3) {
        if (*(char *)(iVar2 + local_58) == DAT_2cff_3c86) {
          local_5a = 1;
        }
        local_58 = local_58 + 1;
      }
    }
    if (local_5a != 0) {
      return DAT_2cff_3c86;
    }
  }
  DAT_2cff_3c86 = 0x44;
  return 0x44;
}


