// FUN_2adf_000c @ 2adf:000c

undefined2 __stdcall16far FUN_2adf_000c(uint param_1)

{
  if ((int)param_1 < 0) {
    if (-param_1 < 0x23 || param_1 == 0xffdd) {
      DAT_2cff_007f = -param_1;
      DAT_2cff_27da = 0xffff;
      return 0xffff;
    }
  }
  else if (param_1 < 0x59) goto LAB_2adf_001f;
  param_1 = 0x57;
LAB_2adf_001f:
  DAT_2cff_27da = param_1;
  DAT_2cff_007f = (int)*(char *)(param_1 + 0x27dc);
  return 0xffff;
}


