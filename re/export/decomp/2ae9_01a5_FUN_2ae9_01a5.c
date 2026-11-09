// FUN_2ae9_01a5 @ 2ae9:01a5

int * __cdecl16far FUN_2ae9_01a5(uint param_1,int param_2)

{
  int *piVar1;
  int in_DX;
  undefined4 local_6;
  
  piVar1 = (int *)FUN_2b1a_00e1(param_1,param_2);
  local_6 = (int *)CONCAT22(in_DX,piVar1);
  if ((in_DX == -1) && (piVar1 == (int *)0xffff)) {
    piVar1 = (int *)0x0;
  }
  else {
    DAT_2cff_2844 = piVar1;
    DAT_2cff_2846 = in_DX;
    DAT_2cff_2848 = piVar1;
    DAT_2cff_284a = in_DX;
    piVar1[1] = param_2 + (uint)(0xfffe < param_1);
    *local_6 = param_1 + 1;
    piVar1 = piVar1 + 4;
  }
  return piVar1;
}


