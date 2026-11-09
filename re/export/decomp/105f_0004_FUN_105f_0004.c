// FUN_105f_0004 @ 105f:0004

void __cdecl16far
FUN_105f_0004(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int in_DX;
  
  iVar1 = FUN_2ae9_000a(0x1000);
  if (iVar1 != 0 || in_DX != 0) {
    iVar2 = FUN_2b4b_000c(param_1,param_2,0x8000);
    if (iVar2 != -1) {
      iVar3 = FUN_2c3b_003d(param_3,param_4,0);
      if (iVar3 != -1) {
        while (iVar4 = FUN_2b58_000d(iVar2,iVar1,in_DX,0x1000), 0 < iVar4) {
          FUN_2b67_0002(iVar3,iVar1,in_DX,iVar4);
        }
        FUN_2b53_0006(iVar3);
      }
      FUN_2b56_0007(iVar2);
    }
    FUN_2c09_0008(iVar1,in_DX);
  }
  return;
}


