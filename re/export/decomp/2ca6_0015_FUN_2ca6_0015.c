// FUN_2ca6_0015 @ 2ca6:0015

uint __cdecl16far FUN_2ca6_0015(void)

{
  uint uVar1;
  
  uVar1 = FUN_1000_0390();
  DAT_2cff_2a44 = DAT_2cff_2a44 + (0xfffe < uVar1);
  DAT_2cff_2a42 = uVar1 + 1;
  return DAT_2cff_2a44 & 0x7fff;
}


