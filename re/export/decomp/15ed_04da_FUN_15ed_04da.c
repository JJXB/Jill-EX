// FUN_15ed_04da @ 15ed:04da

int __cdecl16far FUN_15ed_04da(void)

{
  byte bVar1;
  char cVar2;
  undefined1 uVar4;
  undefined2 uVar3;
  int iVar5;
  undefined1 uVar6;
  bool bVar7;
  
  iVar5 = 0;
  cVar2 = (char)DAT_2cff_0bf6;
  uVar4 = (undefined1)((uint)DAT_2cff_0bf6 >> 8);
  out(CONCAT11(uVar4,cVar2 + '\x06'),0xc6);
  uVar3 = CONCAT11(uVar4,cVar2 + '\n');
  out(uVar3,0);
  bVar1 = in(uVar3);
  uVar6 = bVar1 < 0xc6;
  if (bVar1 == 0xc6) {
    out(CONCAT11(uVar4,cVar2 + '\x06'),0x39);
    uVar3 = CONCAT11(uVar4,cVar2 + '\n');
    out(uVar3,0);
    bVar1 = in(uVar3);
    uVar6 = bVar1 < 0x39;
    if (bVar1 == 0x39) {
      iVar5 = 1;
      goto LAB_15ed_0533;
    }
  }
  bVar7 = false;
  FUN_15ed_0056(0x2cff);
  if (bVar7) {
    uVar6 = 0xf3 < (byte)DAT_2cff_0bf6;
    FUN_15ed_0000();
    if (!(bool)uVar6) {
      FUN_15ed_0000();
      if (!(bool)uVar6) {
        bVar1 = FUN_15ed_0016();
        if ((!(bool)uVar6) && (uVar6 = bVar1 < 0x39, bVar1 == 0x39)) {
          iVar5 = 5;
        }
      }
    }
  }
LAB_15ed_0533:
  FUN_15ed_0314();
  FUN_15ed_0314();
  FUN_15ed_0314();
  FUN_15ed_02f3();
  if (!(bool)uVar6) {
    FUN_15ed_0314();
    FUN_15ed_0314();
    FUN_15ed_02f3();
    if (!(bool)uVar6) {
      FUN_15ed_0314();
      FUN_15ed_0314();
      iVar5 = iVar5 + 2;
    }
  }
  return iVar5;
}


