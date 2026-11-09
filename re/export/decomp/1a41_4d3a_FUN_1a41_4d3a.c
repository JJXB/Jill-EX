// FUN_1a41_4d3a @ 1a41:4d3a

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __cdecl16far FUN_1a41_4d3a(int param_1,int param_2)

{
  int iVar1;
  undefined2 unaff_SS;
  int local_a [4];
  
  FUN_1000_05d8(0x1060,0x2cff,local_a,unaff_SS);
  FUN_1000_05d8(0x1064,0x2cff,local_a + 2,unaff_SS);
  if (param_2 == 0) {
    if (DAT_2cff_3a4d != '\0') {
      for (iVar1 = 0;
          param_2 = local_a[*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f) + 2],
          iVar1 < param_2; iVar1 = iVar1 + 1) {
        FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,
                      local_a[*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_1 * 0x1f)] +
                      DAT_2cff_bbc4 * 0x100 + iVar1 + 0x4000,
                      *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) + iVar1 * 0x10,
                      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f));
      }
    }
  }
  else if (param_2 == 2) {
    if (DAT_2cff_120e == 0) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) =
           *(int *)((int)_DAT_2cff_b7f2 + 8) + DAT_2cff_bab0 + 0x10;
      *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) =
           *(int *)((int)_DAT_2cff_b7f2 + 10) + DAT_2cff_bab2 + 4;
    }
    param_2 = 1;
  }
  return param_2;
}


