// FUN_22ab_1c54 @ 22ab:1c54

void __cdecl16far FUN_22ab_1c54(int param_1)

{
  undefined2 uVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  int unaff_DI;
  undefined2 unaff_SS;
  int local_16;
  char local_14 [16];
  undefined2 local_4;
  
  if (param_1 == 0) {
    local_4 = 8;
  }
  else {
    local_4 = 4;
    for (unaff_DI = 10; 0 < unaff_DI; unaff_DI = unaff_DI + -1) {
      if ((DAT_2cff_83ce <= (int)((undefined2 *)&DAT_2cff_b7cc)[(unaff_DI + -1) * 2]) &&
         ((((undefined2 *)&DAT_2cff_b7cc)[(unaff_DI + -1) * 2] != DAT_2cff_83ce ||
          (DAT_2cff_83cc <= (uint)((undefined2 *)&DAT_2cff_b7ca)[(unaff_DI + -1) * 2])))) break;
    }
    if (unaff_DI < 10) {
      for (iVar5 = 8; uVar4 = DAT_2cff_83cc, unaff_DI <= iVar5; iVar5 = iVar5 + -1) {
        uVar1 = ((undefined2 *)&DAT_2cff_b7ca)[iVar5 * 2];
        ((undefined2 *)&DAT_2cff_b7cc)[(iVar5 + 1) * 2] = ((undefined2 *)&DAT_2cff_b7cc)[iVar5 * 2];
        ((undefined2 *)&DAT_2cff_b7ca)[(iVar5 + 1) * 2] = uVar1;
        FUN_2b9f_0008((undefined1 *)&DAT_2cff_b5ea + (iVar5 + 1) * 10,0x2cff,
                      (undefined1 *)&DAT_2cff_b5ea + iVar5 * 10,0x2cff);
      }
      ((undefined2 *)&DAT_2cff_b7cc)[unaff_DI * 2] = DAT_2cff_83ce;
      ((undefined2 *)&DAT_2cff_b7ca)[unaff_DI * 2] = uVar4;
      ((undefined1 *)&DAT_2cff_b5ea)[unaff_DI * 10] = 0;
    }
    else {
      param_1 = 0;
    }
  }
  FUN_1353_0b64(DAT_2cff_b6ae,DAT_2cff_b6b0,5,local_4);
  FUN_1353_0b4a(DAT_2cff_b6ae,DAT_2cff_b6b0);
  FUN_1353_074e(DAT_2cff_b6ae,DAT_2cff_b6b0,0,4,1,(char *)s______________2cff_13f6,0x2cff);
  FUN_1353_0b64(DAT_2cff_b6ae,DAT_2cff_b6b0,4,local_4);
  FUN_1353_074e(DAT_2cff_b6ae,DAT_2cff_b6b0,4,2,2,(char *)s_HI_SCORES_2cff_1403,0x2cff);
  FUN_1353_0b64(DAT_2cff_b6ae,DAT_2cff_b6b0,2,local_4);
  for (iVar5 = 0; iVar5 < 10; iVar5 = iVar5 + 1) {
    FUN_1353_074e(DAT_2cff_b6ae,DAT_2cff_b6b0,2,iVar5 * 7 + 0xf,2,
                  (undefined1 *)&DAT_2cff_b5ea + iVar5 * 10,0x2cff);
  }
  FUN_1353_0b64(DAT_2cff_b6ae,DAT_2cff_b6b0,6,local_4);
  for (iVar5 = 0; iVar5 < 10; iVar5 = iVar5 + 1) {
    FUN_2b85_00d2(((undefined2 *)&DAT_2cff_b7ca)[iVar5 * 2],
                  ((undefined2 *)&DAT_2cff_b7cc)[iVar5 * 2],local_14);
    for (local_16 = 0; local_14[local_16] != '\0'; local_16 = local_16 + 1) {
      iVar2 = iVar5 * 7 + 0xf;
      iVar3 = FUN_2b9a_000b(local_14);
      FUN_1123_00eb(DAT_2cff_b6ae,DAT_2cff_b6b0,local_14[local_16] + 0x3d0,
                    iVar3 * -4 + 0x3e + local_16 * 4,iVar2);
    }
  }
  if (param_1 != 0) {
    FUN_1353_0b64(DAT_2cff_b6ae,DAT_2cff_b6b0,7,local_4);
    FUN_2b85_0085(((undefined2 *)&DAT_2cff_b7ca)[unaff_DI * 2],local_14);
    iVar5 = FUN_2b9a_000b(local_14);
    if ((iVar5 * -4 + 0x36U) / 6 < 0xc) {
      iVar5 = FUN_2b9a_000b(local_14);
      uVar4 = (iVar5 * -4 + 0x36U) / 6;
    }
    else {
      uVar4 = 0xc;
    }
    FUN_1353_0861(DAT_2cff_b6ae,DAT_2cff_b6b0,2,unaff_DI * 7 + 0xf,2,
                  (undefined1 *)&DAT_2cff_b5ea + unaff_DI * 10,0x2cff,uVar4);
    if (((undefined1 *)&DAT_2cff_b5ea)[unaff_DI * 10] == '\0') {
      FUN_22ab_095b();
    }
    else {
      FUN_22ab_0a39();
    }
    FUN_22ab_1c54(0);
  }
  return;
}


