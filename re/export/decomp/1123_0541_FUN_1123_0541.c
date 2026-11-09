// FUN_1123_0541 @ 1123:0541

void __cdecl16far FUN_1123_0541(undefined2 param_1,undefined2 param_2,int param_3)

{
  if ((DAT_2cff_3a4d & 0xfe) == 0) {
    if (param_3 == -1) {
      FUN_1123_0464(2,2,0xffff);
    }
    else {
      FUN_1123_0464(3,1,0);
    }
  }
  else if ((DAT_2cff_3a4d & 0xfe) == 2) {
    FUN_1123_04eb(param_2,param_1,param_3);
  }
  else if ((DAT_2cff_3a4d & 0xfe) == 4) {
    FUN_1123_0516(param_2,param_1,param_3);
  }
  return;
}


