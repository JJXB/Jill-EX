// FUN_11f4_0dd1 @ 11f4:0dd1

void __cdecl16far FUN_11f4_0dd1(void)

{
  undefined1 *puVar1;
  undefined1 *puVar2;
  int iVar3;
  undefined1 *puVar4;
  undefined1 *puVar5;
  
  if (((DAT_2cff_3a4d & 0xfe) == 2) || ((DAT_2cff_3a4d & 0xfe) == 4)) {
    if ((DAT_2cff_3a4d & 0xfe) == 2) {
      FUN_2ca4_000c(0x3ce,0x105);
    }
    else {
      FUN_2ca4_000c(0x3ce,0x4105);
    }
    FUN_2ca4_000c(0x3c4,0xf02);
    puVar4 = DAT_2cff_3a4b;
    puVar5 = DAT_2cff_3a49;
    for (iVar3 = DAT_2cff_3a47; iVar3 != 0; iVar3 = iVar3 + -1) {
      puVar2 = puVar5;
      puVar5 = puVar5 + 1;
      puVar1 = puVar4;
      puVar4 = puVar4 + 1;
      *puVar2 = *puVar1;
    }
  }
  return;
}


