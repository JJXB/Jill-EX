// FUN_1a41_1e13 @ 1a41:1e13

int __cdecl16far FUN_1a41_1e13(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  
  iVar3 = DAT_2cff_bb98 * 0x100;
  if (param_2 == 0) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) == 1) {
      iVar3 = iVar3 + 1;
    }
    else if (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) == 2) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 1) {
        iVar2 = 2;
      }
      else {
        iVar2 = 3;
      }
      iVar3 = iVar3 + iVar2;
    }
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,iVar3,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    param_2 = (int)(char)((undefined1 *)&DAT_2cff_96ac)[param_3 * 0x1f];
    if ((((undefined2 *)&DAT_2cff_be3c)[param_2] & 0x4000) != 0) {
      FUN_26f4_1fb2(DAT_2cff_bc22,
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      FUN_1a41_09a8(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f),10);
      FUN_26f4_1863(param_1);
      param_2 = FUN_1581_0178(3,0x20);
    }
  }
  else if (param_2 == 2) {
    if ((DAT_2cff_be3a & 3) == 2) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) == 0) {
        iVar3 = FUN_2ca6_0015();
        if (iVar3 % 0x20 != 0) {
          return 0;
        }
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 1;
        FUN_26f4_06b4(param_1,(int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f,0x2cff,
                      (int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f,0x2cff);
      }
      else {
        piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
        *piVar1 = *piVar1 + 1;
        if (2 < *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)) {
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 0;
          FUN_26f4_1892(0x2e,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                             *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) * 8,
                        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
        }
      }
    }
    param_2 = 1;
  }
  return param_2;
}


