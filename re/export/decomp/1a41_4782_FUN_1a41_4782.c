// FUN_1a41_4782 @ 1a41:4782

int __cdecl16far FUN_1a41_4782(int param_1,int param_2)

{
  int iVar1;
  int local_6;
  int local_4;
  
  iVar1 = DAT_2cff_bbbe * 0x100;
  if (param_2 == 0) {
    FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,iVar1,
                  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,iVar1 + 1,
                            *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) + 5,
                            *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) +
                            *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) + 4);
  }
  else if (param_2 == 2) {
    FUN_26f4_263a(0,param_1,&local_6);
    if (local_4 < 2) {
      if (local_4 != 1) {
        local_4 = -1;
      }
    }
    else {
      local_4 = 1;
    }
    if (local_6 < -2) {
      local_6 = -2;
    }
    if ((*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) == local_6) &&
       (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) == local_4)) {
      param_2 = 0;
    }
    else {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) = local_6;
      *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_1 * 0x1f) = local_4;
      param_2 = 1;
    }
  }
  return param_2;
}


