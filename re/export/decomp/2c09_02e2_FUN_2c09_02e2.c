// FUN_2c09_02e2 @ 2c09:02e2

void __cdecl16far FUN_2c09_02e2(int param_1,int param_2)

{
  int iVar1;
  
  if (param_1 != 0 || param_2 != 0) {
    iVar1 = FUN_1000_0314();
    if ((param_2 == DAT_2cff_284a) && (iVar1 == DAT_2cff_2848)) {
      FUN_2c09_0131();
    }
    else {
      FUN_2c09_0216(iVar1,param_2);
    }
  }
  return;
}


