// FUN_15ed_00a5 @ 15ed:00a5

undefined2 __cdecl16near FUN_15ed_00a5(void)

{
  byte bVar1;
  undefined2 uVar2;
  int iVar3;
  
  FUN_15ed_01a8();
  FUN_15ed_01a8();
  FUN_15ed_01a8();
  FUN_15ed_01a8();
  DAT_2cff_0bf8 = '\0';
  FUN_15ed_0034();
  bVar1 = in(0x21);
  out(0x21,bVar1 & 0x53);
  uVar2 = 0;
  iVar3 = 0x800;
  do {
    if (DAT_2cff_0bf8 != '\0') goto LAB_15ed_00fa;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  uVar2 = 3;
LAB_15ed_00fa:
  out(0x21,bVar1);
  FUN_15ed_01d6();
  FUN_15ed_01d6();
  FUN_15ed_01d6();
  FUN_15ed_01d6();
  return uVar2;
}


