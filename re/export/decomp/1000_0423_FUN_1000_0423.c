// FUN_1000_0423 @ 1000:0423

void __cdecl16far FUN_1000_0423(void)

{
  undefined2 uVar1;
  byte extraout_AH;
  
  uVar1 = FUN_1000_0414();
  FUN_1000_0444(uVar1);
  FUN_1000_0414();
  DAT_2cff_2a2f = extraout_AH & 0x7f;
  DAT_2cff_2a2e = extraout_AH & 0x7f;
  return;
}


