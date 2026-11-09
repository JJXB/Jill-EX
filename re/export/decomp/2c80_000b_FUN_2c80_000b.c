// FUN_2c80_000b @ 2c80:000b

void __cdecl16far FUN_2c80_000b(char param_1,char param_2)

{
  byte bVar1;
  byte bVar2;
  
  bVar1 = param_2 + -1 + DAT_2cff_2a2b;
  bVar2 = param_1 + -1 + DAT_2cff_2a2a;
  if ((((DAT_2cff_2a2b <= bVar1) && (bVar1 <= DAT_2cff_2a2d)) && (DAT_2cff_2a2a <= bVar2)) &&
     (bVar2 <= DAT_2cff_2a2c)) {
    FUN_1000_0414();
  }
  return;
}


