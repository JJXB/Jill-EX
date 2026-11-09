// FUN_2ae3_0008 @ 2ae3:0008

void __cdecl16far FUN_2ae3_0008(undefined2 param_1)

{
  int iVar1;
  undefined2 *puVar2;
  undefined2 unaff_BP;
  
  while (iVar1 = DAT_2cff_2842 + -1, DAT_2cff_2842 != 0) {
    puVar2 = (undefined2 *)(iVar1 * 4 + -0x412e);
    DAT_2cff_2842 = iVar1;
    (*(code *)*puVar2)(0x2ae3);
  }
  DAT_2cff_2842 = iVar1;
  (*DAT_2cff_2836)(0x2ae3);
  (*DAT_2cff_283a)(0x2ae3);
  (*DAT_2cff_283e)(0x2ae3);
  FUN_1000_010d(param_1,unaff_BP);
  return;
}


