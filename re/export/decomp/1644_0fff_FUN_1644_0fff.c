// FUN_1644_0fff @ 1644:0fff

int __cdecl16far FUN_1644_0fff(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  int local_6;
  int local_4;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,DAT_2cff_bb42 << 8,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      if (((*(int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) < 1) ||
          (10 < *(int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f))) &&
         (iVar2 = FUN_26f4_1a89(2), iVar2 < 3)) {
        FUN_26f4_1a13(2);
        FUN_1581_0178(2,7);
        FUN_26f4_1863(param_1);
        if (DAT_2cff_0c22 != 0) {
          FUN_22ab_05b4(0xe60,0x2cff,2);
          DAT_2cff_0c22 = 0;
        }
      }
      param_2 = 1;
    }
    else {
      if (((((undefined2 *)&DAT_2cff_be3c)[(char)((undefined1 *)&DAT_2cff_96ac)[param_3 * 0x1f]] &
           0x80) != 0) && (0 < *(int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f))) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = 0;
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0;
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) = 0xf;
        FUN_26f4_0091(param_3);
        FUN_1581_0178(3,10);
      }
      param_2 = 1;
    }
  }
  else if (param_2 == 2) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) < 1) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) == -1) {
        iVar2 = FUN_26f4_1c89(param_1,*(undefined2 *)
                                       ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                              *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 1);
        if (iVar2 == 0) {
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) = 0;
        }
        param_2 = 1;
      }
      else {
        param_2 = 0;
      }
    }
    else {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f);
      *piVar1 = *piVar1 + 1;
      if (0xe < *(int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f)) {
        FUN_26f4_06b4(param_1,&local_6);
        piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
        *piVar1 = *piVar1 + local_6;
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 9) {
          if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < -7) {
            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = 0xfff8;
          }
        }
        else {
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = 8;
        }
        piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
        *piVar1 = *piVar1 + local_4;
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 5) {
          if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < -3) {
            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0xfffc;
          }
        }
        else {
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 4;
        }
      }
      iVar2 = FUN_26f4_1c89(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                    *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                            *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
      if (iVar2 == 0 || 0x40 < *(int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) = 0xffff;
      }
      param_2 = 1;
    }
  }
  return param_2;
}


