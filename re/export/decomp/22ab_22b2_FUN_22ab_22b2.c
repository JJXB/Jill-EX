// FUN_22ab_22b2 @ 22ab:22b2

undefined2 __cdecl16far FUN_22ab_22b2(void)

{
  int iVar1;
  int iVar2;
  undefined2 uVar3;
  undefined1 local_52 [16];
  undefined1 local_42 [32];
  undefined1 local_22 [32];
  
  iVar1 = FUN_22ab_1f69((char *)s_LOAD_GAME_2cff_143f,0x2cff,0x1449,0x2cff);
  if ((iVar1 < 0) ||
     (iVar2 = FUN_2b9a_000b((undefined1 *)&DAT_2cff_bab4 + iVar1 * 0xc,0x2cff), iVar2 == 0)) {
    uVar3 = 0;
  }
  else {
    FUN_2b85_0085(iVar1,local_52);
    FUN_2b9f_0008(local_42);
    FUN_2b96_0009(local_42);
    FUN_2b96_0009(local_42);
    FUN_2b9f_0008(local_22);
    FUN_2b96_0009(local_22);
    FUN_2b96_0009(local_22);
    iVar1 = FUN_2b4b_000c(local_22);
    if (iVar1 != -1) {
      FUN_2b56_0007(iVar1);
      FUN_2b94_000d(0x1456,0x2cff);
      FUN_105f_0004(local_22);
    }
    FUN_22ab_0aa5(local_42);
    uVar3 = 1;
  }
  return uVar3;
}


