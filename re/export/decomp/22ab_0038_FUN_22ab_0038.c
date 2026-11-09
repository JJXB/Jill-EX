// FUN_22ab_0038 @ 22ab:0038

void __cdecl16far FUN_22ab_0038(void)

{
  int iVar1;
  int *piVar2;
  
  iVar1 = 6;
  piVar2 = (int *)((char *)s_Divide_error_Abnormal_program_te_2cff_002f + 0x26);
  do {
    if ((int)DAT_2cff_96ac == *piVar2) {
                    /* WARNING: Could not recover jumptable at 0x00022b01. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*(code *)piVar2[6])();
      return;
    }
    piVar2 = piVar2 + 1;
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  return;
}


