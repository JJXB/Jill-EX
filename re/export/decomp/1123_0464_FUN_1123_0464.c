// FUN_1123_0464 @ 1123:0464

void __cdecl16far FUN_1123_0464(int param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  
  iVar2 = 0;
  for (iVar1 = 0; ((param_1 == iVar1 || (param_2 == iVar1)) || (param_3 == iVar1));
      iVar1 = iVar1 + 1) {
  }
  if (param_3 == -1) {
    for (; ((param_1 == iVar2 || (param_2 == iVar2)) || (iVar1 == iVar2)); iVar2 = iVar2 + 1) {
    }
    FUN_1123_03d0(1,iVar1,param_1,param_2,iVar2,0,3,3,0);
  }
  else {
    FUN_1123_03d0(1,iVar1,param_1,param_2,param_3,0,3,3,3);
  }
  return;
}


