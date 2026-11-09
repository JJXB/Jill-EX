// FUN_150f_0008 @ 150f:0008

void __cdecl16far FUN_150f_0008(undefined2 param_1)

{
  undefined2 uVar1;
  int iVar2;
  int iVar3;
  int local_74;
  undefined1 local_72 [16];
  undefined1 local_62 [32];
  undefined1 local_42 [32];
  undefined1 local_22 [16];
  undefined1 local_12 [16];
  
  FUN_2b85_0085(param_1,local_72);
  FUN_2b9f_0008(local_62);
  FUN_2b96_0009(local_62);
  FUN_2b96_0009(local_62);
  iVar2 = FUN_2c3b_003d(local_62);
  uVar1 = DAT_2cff_3a50;
  if (iVar2 != -1) {
    DAT_2cff_3a50 = DAT_2cff_3a4e;
    for (local_74 = 0; local_74 < 200; local_74 = local_74 + 1) {
      for (iVar3 = 0; iVar3 < 0x140; iVar3 = iVar3 + 1) {
        FUN_12fd_0116(iVar3,local_74);
        FUN_2b67_0002(iVar2,(undefined1 *)&DAT_2cff_3a46,0x2cff,1);
      }
    }
    DAT_2cff_3a50 = uVar1;
    FUN_2b56_0007(iVar2);
  }
  FUN_2b9f_0008(local_62);
  FUN_2b96_0009(local_62);
  FUN_2b96_0009(local_62);
  iVar2 = FUN_2c3b_003d(local_62);
  if (iVar2 != -1) {
    for (iVar3 = 0; iVar3 < 0x100; iVar3 = iVar3 + 1) {
      FUN_2b85_0085((uint)*(byte *)(iVar3 * 3 + 0x194) << 2,local_42);
      FUN_2b85_0085((uint)*(byte *)(iVar3 * 3 + 0x195) << 2,local_22);
      FUN_2b85_0085((uint)*(byte *)(iVar3 * 3 + 0x196) << 2,local_12);
      FUN_2b96_0009(local_42);
      FUN_2b96_0009(local_42);
      FUN_2b96_0009(local_42);
      FUN_2b96_0009(local_42);
      FUN_2b96_0009(local_42);
      FUN_2b9a_000b(local_42);
      FUN_2b67_0002(iVar2,local_42);
    }
    FUN_2b56_0007(iVar2);
  }
  return;
}


