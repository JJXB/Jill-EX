// FUN_22ab_13a1 @ 22ab:13a1

void __cdecl16far FUN_22ab_13a1(void)

{
  bool bVar1;
  int iVar2;
  
  bVar1 = false;
  do {
    FUN_1415_026f(0);
  } while (DAT_2cff_3c86 != 0);
  do {
    FUN_1415_026f(0);
    iVar2 = (DAT_2cff_3e28 - DAT_2cff_bece) / 0x12;
    if ((DAT_2cff_3c86 == 0x1b) || (DAT_2cff_3c86 == 0xd)) {
      bVar1 = true;
    }
    else if ((iVar2 < 2) || ((DAT_2cff_3c86 == 0 && (DAT_2cff_3c7a == 0)))) {
      if (3 < iVar2) {
        bVar1 = true;
      }
    }
    else {
      bVar1 = true;
    }
  } while (!bVar1);
  return;
}


