// FUN_1644_2974 @ 1644:2974

int __cdecl16far FUN_1644_2974(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  undefined2 unaff_SS;
  int local_e [6];
  
  FUN_1000_05d8(0xc62,0x2cff,local_e,unaff_SS);
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            local_e[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) / 2
                                   ] + DAT_2cff_bb4c * 0x100 +
                            (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 0)
                            * 4,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      FUN_26f4_25b8(param_1);
    }
    else if ((((undefined2 *)&DAT_2cff_be3c)[(char)((undefined1 *)&DAT_2cff_96ac)[param_3 * 0x1f]] &
             0x4000) != 0) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) {
        piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f);
        *piVar1 = *piVar1 + 1;
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) < 6) {
          FUN_1a41_09a8(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f),4);
        }
        else {
          FUN_1a41_0930(param_1);
          FUN_26f4_1863(param_1);
        }
      }
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 4;
    }
    param_2 = 1;
  }
  else if (param_2 == 2) {
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
    *piVar1 = *piVar1 + 1;
    if (0xb < *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 0;
    }
    iVar2 = FUN_26f4_1d73(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                  *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                          *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                          *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
    if ((iVar2 == 0) || (iVar2 = FUN_2ca6_0015(), iVar2 % 0x24 == 0)) {
      FUN_26f4_263a(0,param_1,(int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f,0x2cff,
                    (int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f,0x2cff,4);
      FUN_26f4_1892(0x12,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + (DAT_2cff_bb1c + -1) * 0x1f) =
           *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + (DAT_2cff_bb1c + -1) * 0x1f) =
           *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      iVar2 = FUN_2ca6_0015();
      if (iVar2 % 2 == 0) {
        iVar2 = FUN_2ca6_0015();
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = iVar2 % 5 + -2;
        iVar2 = FUN_2ca6_0015();
        *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = iVar2 % 5 + -2;
      }
      else {
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
             *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) / 2;
        *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) =
             *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) / 2;
      }
    }
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f);
    *piVar1 = *piVar1 - (uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f));
    param_2 = 1;
  }
  return param_2;
}


