// FUN_1fae_1d15 @ 1fae:1d15

undefined2 __cdecl16far FUN_1fae_1d15(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  int local_4;
  
  pcVar4 = (char *)0x1fae;
  iVar3 = 3;
  if ((DAT_2cff_3a4d & 0xfe) != 2) {
    iVar3 = 1;
  }
  if (param_2 == 0) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) == 0) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 0) {
        local_4 = 6;
      }
      else {
        local_4 = 4;
      }
      local_4 = (*(uint *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) & 1) + local_4;
    }
    else {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 0) {
        local_4 = 8;
      }
      else {
        local_4 = 0;
      }
      local_4 = (*(uint *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) & 3) + local_4;
    }
    local_4 = local_4 + 0x1000;
    FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,local_4,
                  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                  *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 2);
  }
  else if (param_2 == 2) {
    if ((DAT_2cff_3c76 != 0) || (DAT_2cff_3c78 != 0)) {
      *(uint *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) =
           *(int *)((int)(undefined2 *)&DAT_2cff_96bd + param_1 * 0x1f) + 1U & 3;
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = DAT_2cff_3c76;
      if ((DAT_2cff_3c76 != 0) &&
         (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) != 0)) {
        if ((DAT_2cff_3c76 < 1) ||
           (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) < 0x11)) {
          if ((DAT_2cff_3c76 < 0) &&
             (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) < 0x10)) {
            piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
            *piVar1 = *piVar1 + 1;
            DAT_2cff_3c76 = 0;
          }
          else if (*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) == 0x10) {
            DAT_2cff_3c76 = (iVar3 + 1) * DAT_2cff_3c76;
            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 0;
          }
        }
        else {
          piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
          *piVar1 = *piVar1 + -1;
          DAT_2cff_3c76 = 0;
        }
      }
      *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = DAT_2cff_3c78;
      pcVar4 = (char *)s_JILL_SPLASHES_INTO_THE_UNDERGROU_2cff_26ea + 10;
      iVar2 = FUN_26f4_2227(param_1,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                                    DAT_2cff_3c76 * 2,
                            *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                            DAT_2cff_3c78 * 2,0x200);
      if (iVar2 != 0) {
        piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f);
        *piVar1 = *piVar1 + DAT_2cff_3c76 * 2;
        piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f);
        *piVar1 = *piVar1 + DAT_2cff_3c78 * 2;
      }
    }
    FUN_2cff_2afd(pcVar4,0);
    if (DAT_2cff_bab0 < 1) {
      if (DAT_2cff_bab0 < 0) {
        *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 0x10 - iVar3;
      }
    }
    else {
      *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = iVar3 + 0x10;
    }
    FUN_26f4_23e1(0);
  }
  return 1;
}


