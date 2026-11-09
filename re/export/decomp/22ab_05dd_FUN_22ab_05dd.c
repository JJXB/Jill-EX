// FUN_22ab_05dd @ 22ab:05dd

void __cdecl16far FUN_22ab_05dd(void)

{
  int iVar1;
  undefined2 uVar2;
  undefined2 uVar3;
  undefined1 *puVar4;
  undefined1 local_22 [32];
  
  FUN_1353_0b64(DAT_2cff_b6ae,DAT_2cff_b6b0,0xfff9,8);
  FUN_1123_00eb(DAT_2cff_b6ae,DAT_2cff_b6b0,DAT_2cff_3e26 + 0x60a,0x35,0x2b);
  FUN_1123_00eb(DAT_2cff_b6ae,DAT_2cff_b6b0,DAT_2cff_1210 + 0x60a,0x35,0x4b);
  if (DAT_2cff_83d0 == 0) {
    FUN_1353_0b64(DAT_2cff_b982,DAT_2cff_b984,0xfffb,8);
  }
  else {
    FUN_1353_0b64(DAT_2cff_b982,DAT_2cff_b984,0xfffb,4);
  }
  FUN_1353_0b4a(DAT_2cff_b982,DAT_2cff_b984);
  FUN_1353_074e(DAT_2cff_b982,DAT_2cff_b984,2,2,2,(char *)s_HEALTH_2cff_1325,0x2cff);
  FUN_1353_0b64(DAT_2cff_b982,DAT_2cff_b984,0xfffc,8);
  for (iVar1 = 0; iVar1 < DAT_2cff_83a8 + -1; iVar1 = iVar1 + 1) {
    FUN_1123_00eb(DAT_2cff_b982,DAT_2cff_b984,0xe2a,iVar1 * 3 + 0x2a,2);
  }
  FUN_1123_00eb(DAT_2cff_b982,DAT_2cff_b984,0xe2b,(DAT_2cff_83a8 + -1) * 3 + 0x28,2);
  FUN_1353_074e(DAT_2cff_b982,DAT_2cff_b984,0x21,10,2,(char *)s_SCORE_2cff_132c,0x2cff);
  FUN_2b85_00d2(DAT_2cff_83cc,DAT_2cff_83ce,local_22);
  puVar4 = local_22;
  uVar3 = 2;
  uVar2 = 0x10;
  iVar1 = FUN_2b9a_000b(local_22);
  iVar1 = 0x40 - (iVar1 * 6 + 1);
  FUN_1353_074e(DAT_2cff_b982,DAT_2cff_b984,iVar1,uVar2,uVar3,puVar4);
  FUN_1353_0b64(DAT_2cff_b982,DAT_2cff_b984,0xfffe,8);
  FUN_1353_074e(DAT_2cff_b982,DAT_2cff_b984,1,10,2,(char *)s_LEVEL_2cff_1332,0x2cff);
  if (DAT_2cff_83a6 == 0x7f) {
    FUN_2b9f_0008(local_22);
  }
  else {
    iVar1 = DAT_2cff_83a6 >> 0xf;
    FUN_2b85_00d2(DAT_2cff_83a6,iVar1,local_22);
  }
  FUN_1353_074e(DAT_2cff_b982,DAT_2cff_b984,1,0x10,2,local_22);
  if ((DAT_2cff_120a != 0) && (DAT_2cff_120c == 0)) {
    uVar2 = FUN_2bb2_0553(local_22);
    FUN_2b85_00d2(uVar2,iVar1);
    FUN_2b96_0009(local_22);
    FUN_1353_074e(DAT_2cff_b982,DAT_2cff_b984,0x1c,0x40,2,local_22);
  }
  for (iVar1 = 0; iVar1 < DAT_2cff_83aa; iVar1 = iVar1 + 1) {
    FUN_1123_00eb(DAT_2cff_b982,DAT_2cff_b984,
                  *(int *)(((undefined2 *)&DAT_2cff_83ac)[iVar1] * 2 + 0xed4) + 0xe00,
                  (iVar1 / 3) * 0xe + 1,(iVar1 % 3) * 0xe + 0x1a);
  }
  FUN_22ab_0038();
  FUN_1353_0b4a((undefined2 *)&DAT_2cff_b69e,0x2cff);
  FUN_1353_0b64((undefined2 *)&DAT_2cff_b69e,0x2cff,DAT_2cff_b6b2,0);
  iVar1 = FUN_2b9a_000b((undefined1 *)&DAT_2cff_b662,0x2cff,2,2,(undefined1 *)&DAT_2cff_b662,0x2cff)
  ;
  FUN_1353_074e((undefined2 *)&DAT_2cff_b69e,0x2cff,iVar1 * -3 + 0xa0);
  return;
}


