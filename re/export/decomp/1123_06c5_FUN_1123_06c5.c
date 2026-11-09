// FUN_1123_06c5 @ 1123:06c5

void __cdecl16far FUN_1123_06c5(int param_1)

{
  if ((param_1 == 0) || ((DAT_2cff_3a4d & 0xfe) == 0)) {
    DAT_2cff_3a44 = 0;
    DAT_2cff_3a50 = DAT_2cff_3a4e;
    DAT_2cff_3a49 = DAT_2cff_3a4b;
  }
  else {
    DAT_2cff_3a44 = 1;
    DAT_2cff_3a50 = 1 - DAT_2cff_3a4e;
    FUN_1123_06ac();
    FUN_11f4_0dd1();
  }
  return;
}


