// FUN_1530_0440 @ 1530:0440

void __cdecl16far FUN_1530_0440(int *param_1)

{
  undefined2 uVar1;
  undefined2 in_DX;
  
  DAT_2cff_3e28 = 0;
  DAT_2cff_3e26 = 1;
  FUN_1530_02d7();
  DAT_2cff_3e2c = 0;
  if (*param_1 == 0) {
    DAT_2cff_3e24 = 0;
    *param_1 = 1;
    DAT_2cff_0aac = 1;
  }
  DAT_2cff_3e34 = (char *)s_Yikes__this_game_is_goofed__Plea_2cff_1519 + 0x17;
  DAT_2cff_3e32 = 0x2e7;
  DAT_2cff_0aae = FUN_2c7e_0008(0x1c);
  DAT_2cff_0ab0 = in_DX;
  FUN_2c7e_001a(0x1c,DAT_2cff_3e32,DAT_2cff_3e34);
  DAT_2cff_0aac = 0;
  DAT_2cff_3e36 = *param_1;
  DAT_2cff_3e3a = 0;
  DAT_2cff_3e24 = 1;
  uVar1 = FUN_1000_04f2(0,1,*param_1,*param_1 >> 0xf);
  FUN_1530_03a0(0,2,uVar1);
  return;
}


