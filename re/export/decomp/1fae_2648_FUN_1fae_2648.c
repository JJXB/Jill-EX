// FUN_1fae_2648 @ 1fae:2648

int __cdecl16far FUN_1fae_2648(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  int local_6;
  
  if (param_2 == 0) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96c7 + param_1 * 0x1f) < 1) {
      local_6 = 3;
    }
    else {
      local_6 = 0;
    }
    local_6 = DAT_2cff_bbb0 * 0x100 + local_6;
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 1) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 1) {
        iVar2 = 0;
      }
      else {
        iVar2 = 2;
      }
      local_6 = local_6 + iVar2 + (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f
                                                 ) < 1);
    }
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,local_6,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 2) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) != 0) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c7 + param_1 * 0x1f) =
           *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    }
    if (((*(uint *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) & 7) == 0) &&
       (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) != 0)) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = DAT_2cff_3c76 << 3;
    }
    else {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = DAT_2cff_3c76 << 2;
    }
    iVar2 = FUN_26f4_2227(param_1,*(undefined2 *)
                                   ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                          *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 1,3);
    if (iVar2 == 3) {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      *piVar1 = *piVar1 + 1;
      if (0xc < *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0xc;
      }
    }
    else if ((DAT_2cff_3c7a == 0) && (DAT_2cff_3c7e == 0)) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0xfff8;
      FUN_1581_0178(1,0x12);
    }
    else {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0xfff4;
      FUN_1581_0178(1,0x2b);
    }
    iVar2 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
            *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    uVar4 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
            *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
    uVar3 = FUN_26f4_1c89(param_1,iVar2,uVar4);
    if ((uVar3 & 3) == 0) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 0) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0;
      }
      else {
        FUN_26f4_1c89(param_1,iVar2,
                      ((uVar4 & 0xfff0) + 0x10) -
                      *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f));
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 0;
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0;
      }
    }
    FUN_2cff_2afd((char *)s_JILL_SPLASHES_INTO_THE_UNDERGROU_2cff_26ea + 10,0);
    param_2 = 1;
  }
  return param_2;
}


