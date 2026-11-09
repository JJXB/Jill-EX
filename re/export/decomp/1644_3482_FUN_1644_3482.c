// FUN_1644_3482 @ 1644:3482

int __cdecl16far FUN_1644_3482(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  undefined2 unaff_SS;
  char local_c [10];
  
  if (param_2 == 0) {
    FUN_1353_0b64(DAT_2cff_b7f2,DAT_2cff_b7f4,
                  (*(uint *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f) & 3) + 1,0xffff);
    param_2 = FUN_2b85_0085(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_1 * 0x1f),
                            local_c);
    for (iVar2 = 0; local_c[iVar2] != '\0'; iVar2 = iVar2 + 1) {
      param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,local_c[iVar2] + 0x3d0,
                              *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                              iVar2 * 4,
                              *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
    }
  }
  else if (param_2 == 2) {
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
    iVar2 = *piVar1;
    *piVar1 = *piVar1 + -1;
    if ((SBORROW2(iVar2,1) == *piVar1 < 0) && (iVar2 = FUN_26f4_1da8(param_1), iVar2 != 0)) {
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f);
      *piVar1 = *piVar1 + *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f);
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f);
      *piVar1 = *piVar1 + *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f);
      *piVar1 = *piVar1 + -1;
      param_2 = 1;
    }
    else {
      param_2 = FUN_26f4_1863(param_1);
    }
  }
  return param_2;
}


