// FUN_2c73_0002 @ 2c73:0002

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __cdecl16far FUN_2c73_0002(uint param_1)

{
  int iVar1;
  
  if (DAT_1fae_0520 == 0) {
    iVar1 = _DAT_0000_046c;
    FUN_2c73_005d();
    if (param_1 <= (uint)((iVar1 - _DAT_0000_046c) * -0x37)) {
      return DAT_1fae_0520;
    }
    param_1 = param_1 + (iVar1 - _DAT_0000_046c) * 0x37;
  }
  for (; param_1 != 0; param_1 = param_1 - 1) {
    iVar1 = DAT_1fae_0520;
    do {
      iVar1 = iVar1 + -1;
    } while (iVar1 != 0);
  }
  return DAT_1fae_0520;
}


