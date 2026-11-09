// FUN_1a41_1261 @ 1a41:1261

int __cdecl16far FUN_1a41_1261(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined2 unaff_SS;
  int local_18;
  int local_16;
  int local_12;
  int local_10;
  undefined2 local_e [6];
  
  local_12 = *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
  local_10 = 1;
  FUN_1000_05d8(0xf6c,0x2cff,local_e,unaff_SS);
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            DAT_2cff_bb90 * 0x100 +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      param_2 = FUN_26f4_25b8(param_1);
    }
  }
  else if (param_2 == 2) {
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) =
         local_e[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)];
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f) =
         local_e[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)];
    local_18 = -1;
    local_16 = 0x60;
    for (iVar4 = 0; iVar4 < DAT_2cff_becc; iVar4 = iVar4 + 1) {
      iVar2 = ((undefined2 *)&DAT_2cff_bcac)[iVar4];
      if (((((undefined1 *)&DAT_2cff_96ac)[iVar2 * 0x1f] == ')') &&
          (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) <
           *(int *)((int)(undefined2 *)&DAT_2cff_96bf + iVar2 * 0x1f))) &&
         (iVar3 = FUN_26f4_2772(param_1,iVar2), iVar3 < local_16)) {
        local_18 = iVar2;
        local_16 = iVar3;
      }
    }
    if (-1 < local_18) {
      FUN_26f4_263a(local_18,param_1,&local_12);
    }
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    *piVar1 = *piVar1 + local_12;
    if (0xc < *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f)) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = 0xc;
    }
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < -0xc) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = 0xfff4;
    }
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
    *piVar1 = *piVar1 + local_10;
    if (0xc < *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f)) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0xc;
    }
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < -0xc) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0xfff4;
    }
    iVar4 = FUN_26f4_1d73(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                  *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
    if (iVar4 == 0) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
           -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    }
    iVar4 = FUN_26f4_1d73(param_1,*(undefined2 *)
                                   ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                          *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                          *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
    if ((iVar4 == 0) &&
       (iVar4 = FUN_26f4_1d73(param_1,*(undefined2 *)
                                       ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                              ((*(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                                *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) &
                               0xfff0U) + 0x10) -
                              *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f)),
       iVar4 == 0)) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) =
           -*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      FUN_1581_0178(1,9);
    }
    param_2 = 1;
  }
  return param_2;
}


