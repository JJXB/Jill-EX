// FUN_1fae_202a @ 1fae:202a

int __cdecl16far FUN_1fae_202a(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  undefined2 uVar4;
  int iVar5;
  uint uVar6;
  undefined2 unaff_SS;
  uint local_e;
  int local_a [4];
  
  iVar2 = DAT_2cff_bbaa * 0x100;
  FUN_1000_05d8(0x11f3,0x2cff,local_a,unaff_SS);
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            local_a[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)] +
                            iVar2 + (uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96bb +
                                                       param_1 * 0x1f)) * 3,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 2) {
    *(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) =
         *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) + 1U & 3;
    iVar2 = FUN_26f4_2605(param_1,*(undefined2 *)
                                   ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) != 0) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f) =
           *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    }
    *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = DAT_2cff_3c76 << 3;
    if (iVar2 == 0) {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      *piVar1 = *piVar1 + 2;
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < -0xf) {
        iVar3 = -0x10;
      }
      else {
        iVar3 = *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      }
      if (iVar3 < 0x11) {
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < -0xf) {
          uVar4 = 0xfff0;
        }
        else {
          uVar4 = *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
        }
      }
      else {
        uVar4 = 0x10;
      }
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = uVar4;
    }
    else {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      *piVar1 = *piVar1 + DAT_2cff_3c78 * 2 +
                          (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 2);
      iVar3 = FUN_2ca6_0015();
      if (iVar3 % 4 == 0) {
        FUN_26f4_1892(0x3a,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) + 6,
                      *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + -2);
      }
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < -7) {
        iVar3 = -8;
      }
      else {
        iVar3 = *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      }
      if (iVar3 < 9) {
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < -7) {
          uVar4 = 0xfff8;
        }
        else {
          uVar4 = *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
        }
      }
      else {
        uVar4 = 8;
      }
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = uVar4;
    }
    if ((DAT_2cff_3c7a != 0) && (iVar2 != 0)) {
      DAT_2cff_3d9e = 1;
      FUN_1581_0178(1,0x25);
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0xfffe;
    }
    if (DAT_2cff_3c7e != 0) {
      DAT_2cff_3da0 = 1;
      FUN_1581_0178(1,0x26);
      FUN_26f4_1892(0x3f,*(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                         (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f) < 0) *
                         0x20 + -8,
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + (DAT_2cff_bb1c + -1) * 0x1f) =
           ((uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f)) -
           (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96bb + param_1 * 0x1f) < 0)) * 8 +
           ((uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f)) -
           (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 0)) * 4;
    }
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 9) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < -8) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0xfff8;
      }
    }
    else {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 8;
    }
    iVar2 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f);
    iVar3 = *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
    uVar6 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
            *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
    iVar5 = FUN_26f4_1d73(param_1,*(undefined2 *)
                                   ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),uVar6);
    if (iVar5 == 0) {
      if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 0) {
        local_e = uVar6 + 0x10 & 0xfff0;
      }
      else if (0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f)) {
        local_e = ((uVar6 & 0xfff0) + 0x10) -
                  *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f);
      }
      if ((*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) != 0) &&
         (iVar5 = FUN_26f4_1d73(param_1,*(undefined2 *)
                                         ((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                                local_e), iVar5 == 0)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0;
      }
    }
    FUN_26f4_2605(param_1,iVar2 + iVar3,
                  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
    FUN_2cff_2afd((char *)s_JILL_SPLASHES_INTO_THE_UNDERGROU_2cff_26ea + 10,0);
    param_2 = 1;
  }
  return param_2;
}


