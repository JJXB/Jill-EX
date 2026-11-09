// FUN_1644_0e14 @ 1644:0e14

uint __cdecl16far FUN_1644_0e14(int param_1,uint param_2,int param_3)

{
  int iVar1;
  
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bb40 * 0x100 +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) / 2,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) < 1) {
      if (param_3 == 0) {
        if (DAT_2cff_83a8 < 8) {
          DAT_2cff_83a8 = DAT_2cff_83a8 + 1;
        }
        FUN_26f4_1fb2(DAT_2cff_bbca,
                      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
        DAT_2cff_be38 = DAT_2cff_be38 | 0xc000;
        FUN_1581_0178(2,0xb);
        FUN_26f4_1863(param_1);
        if (DAT_2cff_0c20 != 0) {
          FUN_22ab_05b4(0xe49,0x2cff,2);
          DAT_2cff_0c20 = 0;
        }
        param_2 = 1;
      }
      else {
        param_2 = 0;
      }
    }
    else {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96c9 + param_1 * 0x1f) == 0) {
        FUN_1581_0178(6,0x19);
        FUN_22ab_0f0a(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f));
      }
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c9 + param_1 * 0x1f) = 4;
      FUN_26f4_1863(param_1);
      param_2 = 1;
    }
  }
  else if (param_2 == 2) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 1) {
      iVar1 = -1;
    }
    else {
      iVar1 = 1;
    }
    *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) =
         *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + iVar1 & 7;
    param_2 = (uint)((*(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) & 1) == 0);
  }
  return param_2;
}


