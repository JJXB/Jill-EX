// FUN_1a41_2b1a @ 1a41:2b1a

int __cdecl16far FUN_1a41_2b1a(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int local_c;
  int local_a;
  int local_6 [2];
  
  local_6[0] = 0;
  local_6[1] = 0;
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bba2 * 0x100 +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if ((param_3 == 0) && (7 < *(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f))) {
      param_2 = FUN_26f4_1863(param_1);
    }
    else {
      param_2 = (int)(char)((undefined1 *)&DAT_2cff_96ac)[param_3 * 0x1f];
      if ((((undefined2 *)&DAT_2cff_be3c)[param_2] & 0x400) != 0) {
        FUN_26f4_1863(param_1);
        FUN_26f4_0091(param_3);
        param_2 = FUN_1581_0178(3,10);
      }
    }
  }
  else if (param_2 == 2) {
    *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) =
         ((uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f)) -
         (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 0)) +
         *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) & 3;
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f);
    *piVar1 = *piVar1 + 1;
    if ((*(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f) < 0x40) &&
       (iVar2 = FUN_26f4_1da8(param_1), iVar2 != 0)) {
      local_c = -1;
      local_a = 0x7fff;
      for (iVar2 = 0; iVar2 < DAT_2cff_becc; iVar2 = iVar2 + 1) {
        iVar4 = ((undefined2 *)&DAT_2cff_bcac)[iVar2];
        if (((((((undefined2 *)&DAT_2cff_be3c)[(char)((undefined1 *)&DAT_2cff_96ac)[iVar4 * 0x1f]] &
               0x400) != 0) && (iVar4 != 0)) &&
            (iVar3 = FUN_26f4_2772(param_1,iVar4), iVar3 < local_a)) && (iVar3 < 0x60)) {
          local_c = iVar4;
          local_a = iVar3;
        }
      }
      if (-1 < local_c) {
        FUN_26f4_263a(local_c,param_1,local_6);
      }
      local_6[1] = 1;
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
      *piVar1 = *piVar1 + local_6[0];
      if (0xc < *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = 0xc;
      }
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < -0xc) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = 0xfff4;
      }
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      *piVar1 = *piVar1 + 1;
      if (0xc < *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0xc;
      }
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < -0xc) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0xfff4;
      }
      iVar2 = FUN_26f4_27dd(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                    *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                            *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f);
      *piVar1 = *piVar1 + iVar2 * 10;
      iVar2 = FUN_26f4_1d73(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                    *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      if (iVar2 == 0) {
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
             -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
      }
      iVar4 = ((*(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) & 0xfff0U) + 0x10) -
              DAT_2cff_b7a4;
      iVar2 = FUN_26f4_1d73(param_1,*(undefined2 *)
                                     ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
      if ((iVar2 == 0) &&
         ((FUN_1581_0178(1,0x23),
          *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) == iVar4 ||
          (iVar2 = FUN_26f4_1d73(param_1,*(undefined2 *)
                                          ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),iVar4
                                ), iVar2 == 0)))) {
        *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) =
             -*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      }
      iVar2 = FUN_26f4_1d73(param_1,*(undefined2 *)
                                     ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      if (iVar2 == 0) {
        FUN_26f4_1863(param_1);
      }
      param_2 = 1;
    }
    else {
      FUN_26f4_1863(param_1);
      param_2 = 1;
    }
  }
  return param_2;
}


