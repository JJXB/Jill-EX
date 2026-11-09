// FUN_22ab_124b @ 22ab:124b

void __cdecl16far FUN_22ab_124b(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined2 uVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  undefined1 *puVar8;
  undefined2 uVar9;
  undefined1 local_54 [80];
  int local_4;
  
  DAT_2cff_bece = DAT_2cff_3e28;
  if (param_1 < 0x20) {
    uVar1 = *(undefined4 *)(param_1 * 4 + 0x24a1);
    DAT_2cff_3f0c = (int)((ulong)uVar1 >> 0x10);
    DAT_2cff_3f0a = (int)uVar1;
    DAT_2cff_439a = FUN_2b9a_000b(DAT_2cff_3f0a,DAT_2cff_3f0c);
    if (DAT_2cff_3f0a != 0 || DAT_2cff_3f0c != 0) {
      FUN_1123_06c5(1);
      FUN_1353_023b(0xbc52,0x2cff);
      FUN_1353_0b64(0xbc7a,0x2cff,7,1);
      FUN_1353_0b4a(0xbc7a,0x2cff);
      if ((DAT_2cff_3a4d == '\x04') && (DAT_2cff_17ea != 0)) {
        for (uVar5 = 0; (int)uVar5 < 0x10; uVar5 = uVar5 + 1) {
          FUN_1123_00eb(0xbc8a,0x2cff,DAT_2cff_17ea * 0x100 + uVar5 + 0x4000,(uVar5 & 3) << 4,
                        ((int)uVar5 >> 2) << 4);
        }
      }
      iVar2 = FUN_22ab_0d99();
      local_4 = (DAT_2cff_bc80 + (iVar2 + -1) * -6) / 2;
      for (iVar6 = 0; iVar6 < iVar2; iVar6 = iVar6 + 1) {
        uVar9 = 1;
        uVar3 = FUN_22ab_0dc6(iVar6,local_54);
        FUN_1353_0b64(0xbc7a,0x2cff,uVar3,uVar9);
        puVar8 = local_54;
        uVar3 = 2;
        iVar7 = local_4;
        iVar4 = FUN_2b9a_000b(local_54);
        FUN_1353_074e(0xbc7a,0x2cff,(uint)(DAT_2cff_bc7e + iVar4 * -6) >> 1,iVar7,uVar3,puVar8);
        local_4 = local_4 + 6;
      }
      FUN_1123_0716();
      FUN_22ab_14f6();
    }
  }
  return;
}


