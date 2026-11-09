// FUN_1a41_436d @ 1a41:436d

int __cdecl16far FUN_1a41_436d(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  undefined2 unaff_SS;
  int local_2a [20];
  
  FUN_1000_05d8(0x1038,0x2cff,local_2a,unaff_SS);
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bbba * 0x100 +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if ((((undefined2 *)&DAT_2cff_be3c)[(char)((undefined1 *)&DAT_2cff_96ac)[param_3 * 0x1f]] & 0x80
        ) != 0) {
      FUN_26f4_1863(param_1);
      FUN_26f4_0091(param_3);
      FUN_1581_0178(3,10);
    }
    param_2 = 1;
  }
  else if (param_2 == 2) {
    iVar2 = FUN_26f4_1da8(param_1);
    if (iVar2 == 0) {
      FUN_26f4_1863(param_1);
      param_2 = 1;
    }
    else {
      *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) =
           *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + 1U & 3;
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f);
      *piVar1 = *piVar1 + 1;
      if (0x13 < *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 0;
      }
      FUN_26f4_27dd(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                    *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                    *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
      FUN_26f4_1d73(param_1,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    local_2a[*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f)] +
                    *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                    *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
      iVar2 = FUN_26f4_1d73(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                    *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      if (iVar2 == 0) {
        FUN_26f4_1863(param_1);
      }
      else {
        piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f);
        *piVar1 = *piVar1 + 1;
        if (0x4f < *(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f)) {
          FUN_26f4_1863(param_1);
        }
      }
      param_2 = 1;
    }
  }
  return param_2;
}


