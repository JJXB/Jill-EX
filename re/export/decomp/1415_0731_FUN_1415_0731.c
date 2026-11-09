// FUN_1415_0731 @ 1415:0731

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_1415_0731(void)

{
  char cVar1;
  byte bVar2;
  undefined2 uVar3;
  bool bVar4;
  bool bVar5;
  bool bVar6;
  bool bVar7;
  int local_6;
  
  if (DAT_2cff_3c86 == 0x5b) {
    DAT_2cff_3d96 = 0;
    DAT_2cff_3c86 = 0;
  }
  if (DAT_2cff_3c86 == 0x5d) {
    DAT_2cff_3d96 = 1;
    DAT_2cff_3c86 = 0;
  }
  if (DAT_2cff_3c86 == 0x7d) {
    FUN_1415_06eb();
  }
  else {
    if (DAT_2cff_3d9c == 0) {
      DAT_2cff_04c4 = 0;
      DAT_2cff_04c6 = 0;
      DAT_2cff_04c8 = 0;
      DAT_2cff_04ca = 0;
      DAT_2cff_04cc = DAT_2cff_be3a;
    }
    bVar4 = DAT_2cff_04c4 != DAT_2cff_3c76;
    bVar5 = DAT_2cff_04c6 != DAT_2cff_3c78;
    bVar6 = DAT_2cff_04c8 != DAT_2cff_3c7a;
    bVar7 = DAT_2cff_04ca != DAT_2cff_3c7e;
    if ((DAT_2cff_3c86 < 1) || (0x7f < DAT_2cff_3c86)) {
      cVar1 = '\0';
    }
    else {
      cVar1 = '\x01';
    }
    bVar2 = bVar4 | bVar5 << 1 | bVar6 << 2 | bVar7 << 3 | cVar1 << 4;
    if (bVar2 != 0) {
      if (DAT_2cff_3d9c != 0) {
        if (DAT_2cff_3d96 == 0) {
          local_6 = 1;
        }
        else {
          local_6 = DAT_2cff_be3a - DAT_2cff_04cc;
        }
        uVar3 = (undefined2)((ulong)_DAT_2cff_04c0 >> 0x10);
        if (local_6 < 0x80) {
          *(byte *)((int)_DAT_2cff_04c0 + DAT_2cff_3d9c) = (byte)local_6;
          DAT_2cff_3d9c = DAT_2cff_3d9c + 1;
        }
        else {
          *(byte *)((int)_DAT_2cff_04c0 + DAT_2cff_3d9c) = (byte)local_6 & 0x7f | 0x80;
          DAT_2cff_3d9c = DAT_2cff_3d9c + 1;
          *(undefined1 *)((int)_DAT_2cff_04c0 + DAT_2cff_3d9c) = (char)(local_6 >> 7);
          DAT_2cff_3d9c = DAT_2cff_3d9c + 1;
        }
      }
      *(byte *)((int)_DAT_2cff_04c0 + DAT_2cff_3d9c) = bVar2;
      DAT_2cff_3d9c = DAT_2cff_3d9c + 1;
      if (bVar4) {
        *(undefined1 *)((int)_DAT_2cff_04c0 + DAT_2cff_3d9c) = (undefined1)DAT_2cff_3c76;
        DAT_2cff_3d9c = DAT_2cff_3d9c + 1;
      }
      if (bVar5) {
        *(undefined1 *)((int)_DAT_2cff_04c0 + DAT_2cff_3d9c) = (undefined1)DAT_2cff_3c78;
        DAT_2cff_3d9c = DAT_2cff_3d9c + 1;
      }
      if (bVar6) {
        *(undefined1 *)((int)_DAT_2cff_04c0 + DAT_2cff_3d9c) = (undefined1)DAT_2cff_3c7a;
        DAT_2cff_3d9c = DAT_2cff_3d9c + 1;
      }
      if (bVar7) {
        *(undefined1 *)((int)_DAT_2cff_04c0 + DAT_2cff_3d9c) = (undefined1)DAT_2cff_3c7e;
        DAT_2cff_3d9c = DAT_2cff_3d9c + 1;
      }
      if (cVar1 != '\0') {
        *(undefined1 *)((int)_DAT_2cff_04c0 + DAT_2cff_3d9c) = (undefined1)DAT_2cff_3c86;
        DAT_2cff_3d9c = DAT_2cff_3d9c + 1;
      }
      DAT_2cff_04c4 = DAT_2cff_3c76;
      DAT_2cff_04c6 = DAT_2cff_3c78;
      DAT_2cff_04c8 = DAT_2cff_3c7a;
      DAT_2cff_04ca = DAT_2cff_3c7e;
    }
    if (29999 < DAT_2cff_3d9c) {
      FUN_1415_06eb();
    }
  }
  return;
}


