// FUN_2cf8_0005 @ 2cf8:0005

void __cdecl16far FUN_2cf8_0005(int param_1,int param_2,int param_3,int param_4)

{
  param_1 = param_1 + -1;
  param_3 = param_3 + -1;
  param_2 = param_2 + -1;
  param_4 = param_4 + -1;
  if ((((-1 < param_1) && (param_3 < (int)(uint)DAT_2cff_2a32)) && (-1 < param_2)) &&
     (((param_4 < (int)(uint)DAT_2cff_2a31 && (param_1 <= param_3)) && (param_2 <= param_4)))) {
    DAT_2cff_2a2a = (undefined1)param_1;
    DAT_2cff_2a2c = (undefined1)param_3;
    DAT_2cff_2a2b = (undefined1)param_2;
    DAT_2cff_2a2d = (undefined1)param_4;
    FUN_1000_0414();
  }
  return;
}


