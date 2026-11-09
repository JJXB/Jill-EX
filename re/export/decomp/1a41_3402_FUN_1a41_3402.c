// FUN_1a41_3402 @ 1a41:3402

int __cdecl16far FUN_1a41_3402(int param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int local_e;
  int local_6;
  
  local_e = DAT_2cff_bba8 * 0x100;
  if (param_2 == 0) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 0) {
      local_e = local_e + 1;
    }
    else if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 1) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 0) {
        local_e = local_e + 2;
      }
    }
    else {
      local_e = local_e + 3;
    }
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,local_e,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      param_2 = FUN_26f4_25b8(param_1);
    }
    else {
      param_2 = param_3 * 0x1f;
      if (((undefined1 *)&DAT_2cff_96ac)[param_2] == '>') {
        param_2 = FUN_26f4_1863(param_1);
      }
    }
  }
  else if (param_2 == 2) {
    if (((*(uint *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) & 0xf) == 0) &&
       ((*(uint *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) & 0xf) == 0)) {
      iVar1 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) / 0x10;
      iVar2 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) / 0x10;
      uVar3 = ((undefined2 *)&DAT_2cff_43a6)[iVar1 * 0x40 + iVar2] & 0x3fff;
      iVar6 = (uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f)) -
              (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 0);
      local_6 = (uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f)) -
                (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 0);
      if ((iVar6 == 0) && (local_6 == 0)) {
        iVar4 = FUN_2ca6_0015();
        if (iVar4 % 2 == 0) {
          iVar6 = 1;
        }
        else {
          local_6 = 1;
        }
      }
      if ((((undefined2 *)&DAT_2cff_43a6)[(iVar1 + iVar6) * 0x40 + iVar2 + local_6] & 0x3fff) !=
          uVar3) {
        iVar4 = FUN_2ca6_0015();
        iVar5 = (iVar4 % 2) * 2 + -1;
        iVar4 = iVar6 * iVar5;
        iVar6 = local_6 * iVar5;
        local_6 = iVar4;
        if ((((undefined2 *)&DAT_2cff_43a6)[(iVar1 + iVar6) * 0x40 + iVar2 + iVar4] & 0x3fff) !=
            uVar3) {
          iVar6 = -iVar6;
          local_6 = -iVar4;
          if ((((undefined2 *)&DAT_2cff_43a6)[(iVar1 + iVar6) * 0x40 + iVar2 + -iVar4] & 0x3fff) !=
              uVar3) {
            iVar6 = -((uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f)) -
                     (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 0));
            local_6 = -((uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f)) -
                       (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 0));
            if ((((undefined2 *)&DAT_2cff_43a6)[(iVar1 + iVar6) * 0x40 + iVar2 + local_6] & 0x3fff)
                != uVar3) {
              iVar6 = 0;
              local_6 = 0;
            }
          }
        }
      }
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) =
           *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) * iVar6;
      *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) =
           *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) * local_6;
    }
    FUN_26f4_1c89(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                          *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f),
                  *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                  *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f));
    param_2 = 1;
  }
  return param_2;
}


