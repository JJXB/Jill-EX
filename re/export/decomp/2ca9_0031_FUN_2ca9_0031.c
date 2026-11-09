// FUN_2ca9_0031 @ 2ca9:0031

void __stdcall16far
FUN_2ca9_0031(char param_1,char param_2,char param_3,char param_4,char param_5,char param_6)

{
  byte bVar1;
  byte bVar2;
  undefined2 unaff_SS;
  undefined1 local_a2 [160];
  
  if (((DAT_2cff_2a33 == '\0') && (DAT_2cff_2a39 != 0)) && (param_1 == '\x01')) {
    param_5 = param_5 + '\x01';
    bVar1 = param_4 + 1;
    param_3 = param_3 + '\x01';
    bVar2 = param_2 + 1;
    if (param_6 == '\x06') {
      FUN_2c9a_0000(param_5,bVar1 + 1,param_3,bVar2,param_5,bVar1);
      FUN_2c86_0006(param_5,bVar2,param_5,bVar2,local_a2);
      FUN_2ca9_000d(param_3,param_5,local_a2,unaff_SS);
      FUN_2c86_005d(param_5,bVar2,param_3,bVar2,local_a2);
    }
    else {
      FUN_2c9a_0000(param_5,bVar1,param_3,bVar2 - 1,param_5,bVar1 + 1);
      FUN_2c86_0006(param_5,bVar1,param_5,bVar1,local_a2);
      FUN_2ca9_000d(param_3,param_5,local_a2,unaff_SS);
      FUN_2c86_005d(param_5,bVar1,param_3,bVar1,local_a2);
    }
  }
  else {
    FUN_1000_0414();
  }
  return;
}


