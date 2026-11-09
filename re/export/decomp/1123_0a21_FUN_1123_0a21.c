// FUN_1123_0a21 @ 1123:0a21

undefined2 __cdecl16far FUN_1123_0a21(void)

{
  undefined2 uVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  
  FUN_2c66_0006(0x49e,0x2cff);
  do {
    uVar1 = FUN_1333_0066();
    iVar2 = FUN_2ce7_000d(uVar1);
    if (((iVar2 == 0x43) || (iVar2 == 0x45)) || (iVar2 == 0x56)) break;
  } while (iVar2 != 0x1b);
  iVar3 = 4;
  piVar4 = (int *)((char *)s_graphics_are_recommended__2cff_0a67 + 3);
  do {
    if (iVar2 == *piVar4) {
                    /* WARNING: Could not recover jumptable at 0x00011c96. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      uVar1 = (*(code *)piVar4[4])();
      return uVar1;
    }
    piVar4 = piVar4 + 1;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  return 1;
}


