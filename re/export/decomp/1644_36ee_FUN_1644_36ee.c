// FUN_1644_36ee @ 1644:36ee

uint __cdecl16far FUN_1644_36ee(int param_1,uint param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  uint uVar3;
  int local_8;
  int local_6;
  uint local_4;
  
  if (param_2 == 0) {
    local_8 = ((undefined2 *)&DAT_2cff_bb3e)[(char)((undefined1 *)&DAT_2cff_96ac)[param_1 * 0x1f]] *
              0x100 + (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) < 0) * 3;
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 1) {
      local_8 = local_8 + (uint)(0 < *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f)) *
                          2 + (uint)(*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) <
                                    1);
    }
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,local_8,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 1) {
    if (param_3 == 0) {
      param_2 = FUN_26f4_25b8(param_1);
    }
  }
  else if (param_2 == 2) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
      *piVar1 = *piVar1 + 1;
      iVar2 = *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
      if (0x10 < iVar2) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 1;
        FUN_26f4_06b4(param_1,&local_6);
        *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = local_6 << 2;
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0xfff6;
      }
      param_2 = (uint)(0x10 < iVar2);
    }
    else {
      param_2 = 1;
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      *piVar1 = *piVar1 + 1;
      if (0xc < *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f)) {
        *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 10;
      }
      local_6 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
      local_4 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      uVar3 = FUN_26f4_1c89(param_1,local_6,local_4);
      if ((uVar3 & 3) == 0) {
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) < 0) {
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = 0;
        }
        else {
          local_4 = ((local_4 & 0xfff0) + 0x10) -
                    *(int *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f);
          FUN_26f4_1c89(param_1,local_6,local_4);
          FUN_26f4_06b4(param_1,&local_6);
          *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = local_6;
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) = 0;
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) = 0;
          FUN_1581_0178(1,0x12);
        }
      }
    }
  }
  return param_2;
}


