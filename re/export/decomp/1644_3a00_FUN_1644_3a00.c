// FUN_1644_3a00 @ 1644:3a00

uint __cdecl16far FUN_1644_3a00(int param_1,uint param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int local_6;
  
  iVar2 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) / 0x10;
  iVar3 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) / 0x10;
  if (param_2 == 0) {
    if (DAT_2cff_120e != 0) {
      FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,0xe05,
                    *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) + 4,
                    *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) + 0xc);
    }
    param_2 = param_1 * 0x1f;
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_2) != 0) {
      FUN_22ab_140b(iVar2,iVar3);
      FUN_22ab_140b(iVar2,iVar3 + 1);
      FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,DAT_2cff_88f4,
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) -
                    *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f));
      FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,DAT_2cff_88fc,
                    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                    *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                    *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) + 0x10);
      FUN_22ab_140b(iVar2,iVar3 + -1);
      param_2 = FUN_22ab_140b(iVar2,iVar3 + 2);
    }
  }
  else if (param_2 == 2) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) {
      param_2 = 0;
    }
    else {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f);
      *piVar1 = *piVar1 + 1;
      if (0x10 < *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f)) {
        FUN_26f4_1863(param_1);
      }
      param_2 = 1;
    }
  }
  else if (param_2 == 3) {
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f) == 0) {
      if ((((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar3] & 0x3fff) == 0xbe) {
        iVar4 = FUN_26f4_1a38(3);
        if (iVar4 == 0) {
          param_2 = 0;
          if (DAT_2cff_0c28 != 0) {
            param_2 = FUN_22ab_05b4(0xe93,0x2cff,1);
            DAT_2cff_0c28 = 0;
          }
        }
        else {
          FUN_1581_0178(3,0x24);
          if (DAT_2cff_0c26 != 0) {
            DAT_2cff_0c26 = 0;
            FUN_22ab_05b4(0xe84,0x2cff,1);
          }
          ((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar3] =
               ((undefined2 *)&DAT_2cff_43a6)
               [(*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) + iVar2) * 0x40 +
                *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) + iVar3] & 0x3fff |
               0xc000;
          param_2 = FUN_26f4_1863(param_1);
        }
      }
      else {
        iVar4 = FUN_26f4_1a38(1);
        if (iVar4 == 0) {
          param_2 = 0;
          if (DAT_2cff_0c1c != 0) {
            DAT_2cff_0c1c = 0;
            param_2 = FUN_22ab_05b4(0xeb9,0x2cff,2);
          }
        }
        else {
          if (DAT_2cff_0c1e != 0) {
            DAT_2cff_0c1e = 0;
            FUN_22ab_05b4(0xeaa,0x2cff,1);
          }
          FUN_1581_0178(3,0xc);
          param_2 = param_1 * 0x1f;
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_2) = 1;
          for (local_6 = 0; local_6 < 2; local_6 = local_6 + 1) {
            param_2 = ((undefined2 *)&DAT_2cff_43a6)[(iVar2 + -1) * 0x40 + iVar3 + local_6] & 0x3fff
                      | 0xc000;
            ((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar3 + local_6] = param_2;
          }
        }
      }
    }
    else {
      param_2 = 0;
    }
  }
  return param_2;
}


