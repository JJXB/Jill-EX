// FUN_26f4_0d0c @ 26f4:0d0c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_26f4_0d0c(int param_1,int param_2)

{
  int *piVar1;
  
  FUN_11f4_0e4c(DAT_2cff_b7f2,DAT_2cff_b7f4,-param_1,-param_2);
  piVar1 = (int *)((int)_DAT_2cff_b7f2 + 8);
  *piVar1 = *piVar1 + param_1;
  piVar1 = (int *)((int)_DAT_2cff_b7f2 + 10);
  *piVar1 = *piVar1 + param_2;
  FUN_26f4_0752(param_1,param_2,0xc000);
  return;
}


