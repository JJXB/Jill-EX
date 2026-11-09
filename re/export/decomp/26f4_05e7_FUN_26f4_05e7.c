// FUN_26f4_05e7 @ 26f4:05e7

void __cdecl16far FUN_26f4_05e7(int param_1,int param_2)

{
  int iVar1;
  
  if ((DAT_2cff_96ac != '\x17') &&
     ((DAT_2cff_96ac != '\0' || ((((undefined2 *)&DAT_2cff_be2c)[DAT_2cff_96b9] & 2) == 0)))) {
    iVar1 = FUN_26f4_1a89(10);
    if (0 < param_1 - iVar1) {
      DAT_2cff_be38 = DAT_2cff_be38 | 0xc000;
      DAT_2cff_83a8 = DAT_2cff_83a8 - (param_1 - iVar1);
      DAT_2cff_83d0 = 1;
      if (DAT_2cff_83a8 < 1) {
        DAT_2cff_83a8 = 0;
        DAT_2cff_96ac = '\0';
        DAT_2cff_96b5 = 0x10;
        DAT_2cff_96b7 = 0x20;
        DAT_2cff_96b9 = 5;
        DAT_2cff_96bd = 0;
        DAT_2cff_96bb = param_2;
        if (param_2 == 1) {
          DAT_2cff_96af = DAT_2cff_96af - 1 & 0xfff0;
        }
        DAT_2cff_96b3 = 0xfff4;
        FUN_1581_0178(4,param_2 + 0x27);
        FUN_1a41_09a8(DAT_2cff_96ad,DAT_2cff_96af,10);
      }
      else {
        FUN_1581_0178(4,0x13);
      }
    }
  }
  return;
}


