// FUN_1415_093b @ 1415:093b

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_1415_093b(void)

{
  byte bVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  undefined2 uVar5;
  
  iVar3 = FUN_1333_000f();
  if ((iVar3 != 0) &&
     ((iVar3 = FUN_1333_0066(), DAT_2cff_3dca == 0 || ((DAT_2cff_3dca == 1 && (iVar3 == 0x1b)))))) {
    FUN_1415_05b2();
    DAT_2cff_3dcc = 1;
  }
  DAT_2cff_3c86 = 0;
  if (DAT_2cff_3d9c == 0) {
    DAT_2cff_3c76 = 0;
    DAT_2cff_3c78 = 0;
    DAT_2cff_3c7a = 0;
    DAT_2cff_3c7e = 0;
    DAT_2cff_3c72 = DAT_2cff_be3a;
    DAT_2cff_3c74 = 0;
  }
  uVar2 = DAT_2cff_3d9c;
  if ((int)DAT_2cff_3c74 <= DAT_2cff_be3a - DAT_2cff_3c72) {
    uVar5 = (undefined2)((ulong)_DAT_2cff_04c0 >> 0x10);
    iVar4 = (int)_DAT_2cff_04c0;
    bVar1 = *(byte *)(iVar4 + DAT_2cff_3d9c);
    iVar3 = DAT_2cff_3d9c + 1;
    if ((bVar1 & 1) != 0) {
      DAT_2cff_3c76 = (int)*(char *)(iVar4 + DAT_2cff_3d9c + 1);
      iVar3 = DAT_2cff_3d9c + 2;
    }
    DAT_2cff_3d9c = iVar3;
    if ((bVar1 & 2) != 0) {
      DAT_2cff_3c78 = (int)*(char *)(iVar4 + DAT_2cff_3d9c);
      DAT_2cff_3d9c = DAT_2cff_3d9c + 1;
    }
    if ((bVar1 & 4) != 0) {
      DAT_2cff_3c7a = (int)*(char *)(iVar4 + DAT_2cff_3d9c);
      DAT_2cff_3d9c = DAT_2cff_3d9c + 1;
    }
    if ((bVar1 & 8) != 0) {
      DAT_2cff_3c7e = (int)*(char *)(iVar4 + DAT_2cff_3d9c);
      DAT_2cff_3d9c = DAT_2cff_3d9c + 1;
    }
    if ((bVar1 & 0x10) != 0) {
      DAT_2cff_3c86 = (int)*(char *)(iVar4 + DAT_2cff_3d9c);
      DAT_2cff_3d9c = DAT_2cff_3d9c + 1;
    }
    DAT_2cff_3c74 = (uint)*(char *)(iVar4 + DAT_2cff_3d9c);
    uVar2 = DAT_2cff_3d9c + 1;
    if ((int)DAT_2cff_3c74 < 0) {
      DAT_2cff_3c74 = *(char *)(iVar4 + DAT_2cff_3d9c + 1) * 0x80 + (DAT_2cff_3c74 & 0x7f);
      uVar2 = DAT_2cff_3d9c + 2;
    }
  }
  DAT_2cff_3d9c = uVar2;
  if (DAT_2cff_3d98 <= DAT_2cff_3d9c) {
    FUN_1415_05b2();
  }
  return;
}


