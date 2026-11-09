// FUN_2ae9_0137 @ 2ae9:0137

int * __cdecl16far FUN_2ae9_0137(uint param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  int in_DX;
  undefined4 local_6;
  
  piVar1 = (int *)FUN_2b1a_00e1(param_1,param_2);
  local_6 = (int *)CONCAT22(in_DX,piVar1);
  if ((in_DX == -1) && (piVar1 == (int *)0xffff)) {
    piVar2 = (int *)0x0;
  }
  else {
    piVar1[3] = DAT_2cff_284a;
    piVar1[2] = (int)DAT_2cff_2848;
    piVar1[1] = param_2 + (uint)(0xfffe < param_1);
    *local_6 = param_1 + 1;
    piVar2 = piVar1 + 4;
    DAT_2cff_2848 = piVar1;
    DAT_2cff_284a = in_DX;
  }
  return piVar2;
}


