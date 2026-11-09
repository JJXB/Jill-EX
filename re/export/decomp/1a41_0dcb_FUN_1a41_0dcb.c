// FUN_1a41_0dcb @ 1a41:0dcb

int __cdecl16far FUN_1a41_0dcb(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  undefined2 unaff_SS;
  int local_1c [4];
  int local_14 [4];
  int local_c [4];
  int local_4;
  
  FUN_1000_05d8(0xf54,0x2cff,local_1c,unaff_SS);
  FUN_1000_05d8(0xf5c,0x2cff,local_14,unaff_SS);
  FUN_1000_05d8(0xf64,0x2cff,local_c,unaff_SS);
  local_4 = DAT_2cff_bb8c * 0x100;
  if (param_2 == 0) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 1) {
      FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                    local_1c[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)] + local_4
                    + 9,*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      for (iVar2 = 1;
          iVar2 <= (*(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) + -0x18) / 8;
          iVar2 = iVar2 + 1) {
        FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                      local_14[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)] +
                      local_4,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                              (iVar2 + 1) * 8,
                      *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 0xc);
      }
      param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                              local_c[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)]
                              + local_4 + 5,
                              *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                              *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) + -8,
                              *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 0xc);
    }
    else {
      FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                    local_1c[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)] + local_4
                    ,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                     *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) + -0x10,
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      for (iVar2 = 1;
          iVar2 <= (*(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) + -0x18) / 8;
          iVar2 = iVar2 + 1) {
        FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                      local_14[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)] +
                      local_4,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                              iVar2 * 8,
                      *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 0xc);
      }
      param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                              local_c[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)]
                              + local_4,
                              *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                              *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 0xc);
    }
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      FUN_26f4_25b8(param_1);
      param_2 = 1;
    }
    else {
      param_2 = (int)(char)((undefined1 *)&DAT_2cff_96ac)[param_3 * 0x1f];
      if (((((undefined2 *)&DAT_2cff_be3c)[param_2] & 0x4000) != 0) &&
         (param_2 = param_1 * 0x1f, *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_2) == 0)) {
        FUN_26f4_012d(param_1);
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 0x10;
        piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f);
        *piVar1 = *piVar1 + -8;
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) < 0x18) {
          param_2 = FUN_26f4_0091(param_1);
        }
        else {
          param_2 = FUN_1581_0178(3,0x1f);
        }
      }
    }
  }
  else if (param_2 == 2) {
    if (0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f)) {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f);
      *piVar1 = *piVar1 + -1;
    }
    *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) =
         *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + 1U & 3;
    iVar2 = FUN_26f4_1f35(param_1,*(undefined2 *)
                                   ((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),0);
    if (iVar2 == 0) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
           -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    }
    param_2 = 1;
  }
  return param_2;
}


