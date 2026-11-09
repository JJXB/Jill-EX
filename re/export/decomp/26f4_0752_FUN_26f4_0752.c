// FUN_26f4_0752 @ 26f4:0752

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_26f4_0752(int param_1,int param_2,uint param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined2 uVar5;
  int local_e;
  
  iVar1 = (int)_DAT_2cff_b7f2;
  uVar5 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
  if (param_1 < 1) {
    if (param_1 < 0) {
      iVar1 = *(int *)(iVar1 + 8) / 0x10;
      for (local_e = 0; local_e < DAT_2cff_b99a + 1; local_e = local_e + 1) {
        iVar4 = *(int *)((int)_DAT_2cff_b7f2 + 10) / 0x10 + local_e;
        ((undefined2 *)&DAT_2cff_43a6)[iVar1 * 0x40 + iVar4] =
             ((undefined2 *)&DAT_2cff_43a6)[iVar1 * 0x40 + iVar4] | param_3;
        if (param_3 == 0x4000) {
          FUN_22ab_140b(iVar1,iVar4);
        }
      }
    }
  }
  else {
    iVar4 = ((*(int *)(iVar1 + 8) + *(int *)(iVar1 + 4)) - param_1) / 0x10;
    if ((*(int *)(iVar1 + 8) + *(int *)(iVar1 + 4) + -1) / 0x10 < 0x7f) {
      iVar1 = (*(int *)(iVar1 + 8) + *(int *)(iVar1 + 4) + -1) / 0x10;
    }
    else {
      iVar1 = 0x7f;
    }
    for (; iVar4 <= iVar1; iVar4 = iVar4 + 1) {
      for (local_e = 0; local_e < DAT_2cff_b99a + 1; local_e = local_e + 1) {
        uVar5 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
        if (*(int *)((int)_DAT_2cff_b7f2 + 10) / 0x10 + local_e < 0x3f) {
          iVar3 = *(int *)((int)_DAT_2cff_b7f2 + 10) / 0x10 + local_e;
        }
        else {
          iVar3 = 0x3f;
        }
        ((undefined2 *)&DAT_2cff_43a6)[iVar4 * 0x40 + iVar3] =
             ((undefined2 *)&DAT_2cff_43a6)[iVar4 * 0x40 + iVar3] | param_3;
        if (param_3 == 0x4000) {
          FUN_22ab_140b(iVar4,iVar3);
        }
      }
    }
  }
  iVar1 = (int)_DAT_2cff_b7f2;
  uVar5 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
  if (param_2 < 1) {
    if (param_2 < 0) {
      for (iVar1 = *(int *)(iVar1 + 10) / 0x10;
          iVar1 <= ((*(int *)((int)_DAT_2cff_b7f2 + 10) - param_2) + -1) / 0x10; iVar1 = iVar1 + 1)
      {
        for (iVar4 = 0; iVar4 < DAT_2cff_b998 + 1; iVar4 = iVar4 + 1) {
          uVar5 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
          if (*(int *)((int)_DAT_2cff_b7f2 + 8) / 0x10 + iVar4 < 0x7f) {
            iVar3 = *(int *)((int)_DAT_2cff_b7f2 + 8) / 0x10 + iVar4;
          }
          else {
            iVar3 = 0x7f;
          }
          ((undefined2 *)&DAT_2cff_43a6)[iVar3 * 0x40 + iVar1] =
               ((undefined2 *)&DAT_2cff_43a6)[iVar3 * 0x40 + iVar1] | param_3;
          if (param_3 == 0x4000) {
            FUN_22ab_140b(iVar3,iVar1);
          }
        }
      }
    }
  }
  else {
    iVar1 = *(int *)(iVar1 + 10) + *(int *)(iVar1 + 6);
    if ((iVar1 + -1) / 0x10 < 0x3f) {
      iVar4 = (iVar1 + -1) / 0x10;
    }
    else {
      iVar4 = 0x3f;
    }
    for (iVar1 = (iVar1 - param_2) / 0x10; iVar1 <= iVar4; iVar1 = iVar1 + 1) {
      for (iVar3 = 0; iVar3 < DAT_2cff_b998 + 1; iVar3 = iVar3 + 1) {
        uVar5 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
        if (*(int *)((int)_DAT_2cff_b7f2 + 8) / 0x10 + iVar3 < 0x7f) {
          iVar2 = *(int *)((int)_DAT_2cff_b7f2 + 8) / 0x10 + iVar3;
        }
        else {
          iVar2 = 0x7f;
        }
        ((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar1] =
             ((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar1] | param_3;
        if (param_3 == 0x4000) {
          FUN_22ab_140b(iVar2,iVar1);
        }
      }
    }
  }
  return;
}


