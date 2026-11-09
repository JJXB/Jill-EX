// FUN_1581_050c @ 1581:050c

undefined2 __cdecl16far FUN_1581_050c(undefined2 param_1,undefined2 param_2)

{
  int iVar1;
  int in_DX;
  undefined2 uVar2;
  int local_8;
  undefined2 local_6;
  
  uVar2 = 0;
  if (DAT_2cff_0bf0 != 0 || DAT_2cff_0bf2 != 0) {
    FUN_2c09_0008(DAT_2cff_0bf0,DAT_2cff_0bf2);
    DAT_2cff_0bf2 = 0;
    DAT_2cff_0bf0 = 0;
  }
  local_8 = FUN_2b31_003f(param_1,param_2,1,&local_8);
  if (-1 < local_8) {
    local_6 = FUN_2c44_0001(local_8);
    iVar1 = FUN_2ae9_000a(local_6);
    if (iVar1 != 0 || in_DX != 0) {
      uVar2 = 1;
      DAT_2cff_0bf0 = iVar1;
      DAT_2cff_0bf2 = in_DX;
      iVar1 = FUN_2b65_0002(local_8,iVar1,in_DX,0x8000);
      if (iVar1 < 1) {
        uVar2 = 0;
      }
    }
    FUN_2b56_0007(local_8);
  }
  return uVar2;
}


