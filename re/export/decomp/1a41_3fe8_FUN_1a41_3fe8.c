// FUN_1a41_3fe8 @ 1a41:3fe8

int __cdecl16far FUN_1a41_3fe8(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar2 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) >> 4;
  iVar4 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) >> 4;
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,0xe2c,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (((undefined1 *)&DAT_2cff_96ac)[param_3 * 0x1f] == '\0') {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 6;
      if (DAT_2cff_0ece != 0) {
        DAT_2cff_0ece = 0;
        FUN_22ab_05b4((char *)s_Press_UP_DOWN_to_use_elevator_2cff_116a,0x2cff,2);
      }
      if (DAT_2cff_3c78 < 0) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_3 * 0x1f) = 0;
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f) != DAT_2cff_3c78) {
          FUN_1581_0178(2,0x1d);
        }
        iVar3 = FUN_26f4_1d73(0,DAT_2cff_96ad,(iVar4 + -2) * 0x10);
        if (iVar3 != 0) {
          FUN_26f4_1ad4(param_1,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                        *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + -0x10);
          ((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar4] = 0xc08a;
        }
      }
      else if (0 < DAT_2cff_3c78) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_3 * 0x1f) = 0;
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f) != DAT_2cff_3c78) {
          FUN_1581_0178(2,0x1e);
        }
        if ((((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar4 + 1] & 0x3fff) == 0x8a) {
          ((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar4 + 1] =
               ((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar4] & 0x3fff | 0xc000;
          FUN_26f4_1ad4(param_1,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                        *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 0x10);
          FUN_26f4_1d73(0,DAT_2cff_96ad,iVar4 << 4);
        }
      }
      *(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f) = DAT_2cff_3c78;
    }
    param_2 = 1;
  }
  else if (param_2 == 2) {
    if (0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f)) {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f);
      *piVar1 = *piVar1 + -1;
    }
    if (((*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) &&
        ((((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar4 + 1] & 0x3fff) == 0x8a)) &&
       (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) != -1)) {
      ((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar4 + 1] =
           ((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar4] & 0x3fff | 0xc000;
      FUN_26f4_1ad4(param_1,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 0x10);
      param_2 = 1;
    }
    else {
      param_2 = 0;
    }
  }
  return param_2;
}


