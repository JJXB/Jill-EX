// FUN_22ab_2600 @ 22ab:2600

void __cdecl16far FUN_22ab_2600(void)

{
  int iVar1;
  undefined2 unaff_SS;
  char local_36 [52];
  
  FUN_1000_05d8(0x1246,0x2cff,local_36,unaff_SS);
  DAT_2cff_3d9e = 0;
  do {
    FUN_1581_063e();
    FUN_1415_059e();
    DAT_2cff_3c86 = FUN_2ce7_000d(DAT_2cff_3c86);
    for (iVar1 = 0; local_36[iVar1] != '\0'; iVar1 = iVar1 + 1) {
      if (local_36[iVar1] == DAT_2cff_3c86) {
        FUN_1581_0178(1,iVar1 + 1);
      }
    }
  } while ((DAT_2cff_3c86 != 0xd) && (DAT_2cff_3c86 != 0x1b));
  return;
}


