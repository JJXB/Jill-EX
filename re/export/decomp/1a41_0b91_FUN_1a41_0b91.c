// FUN_1a41_0b91 @ 1a41:0b91

int __cdecl16far FUN_1a41_0b91(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  undefined2 unaff_SS;
  int local_14 [9];
  
  FUN_1000_05d8(0xf42,0x2cff,local_14,unaff_SS);
  if (param_2 == 0) {
    param_2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                            local_14[*(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)] +
                            DAT_2cff_bb88 * 0x100,
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f),
                            *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
  }
  else if (param_2 == 2) {
    piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f);
    *piVar1 = *piVar1 + 1;
    if ((8 < *(int *)((int)(undefined2 *)&DAT_2cff_96bf + param_1 * 0x1f)) ||
       (iVar2 = FUN_26f4_1da8(param_1), iVar2 == 0)) {
      FUN_26f4_1863(param_1);
    }
    param_2 = 1;
  }
  return param_2;
}


