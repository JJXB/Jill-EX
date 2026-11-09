// FUN_1581_05ba @ 1581:05ba

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_1581_05ba(void)

{
  int iVar1;
  int iVar2;
  undefined2 uVar3;
  undefined1 extraout_AH;
  int iVar4;
  int iVar5;
  undefined2 uVar6;
  
  uVar6 = (undefined2)((ulong)_DAT_2cff_0bf0 >> 0x10);
  iVar4 = (int)_DAT_2cff_0bf0;
  iVar5 = iVar4 + *(int *)(iVar4 + 6);
  iVar1 = *(int *)(iVar4 + 8);
  FUN_2980_007d();
  iVar2 = *(int *)((int)_DAT_2cff_0bf0 + 0xc);
  uVar3 = FUN_1000_04f2(0x34dc,0x12,iVar2,iVar2 >> 0xf);
  FUN_2980_009a(uVar3);
  if (iVar5 != 0) {
    FUN_2980_0067(iVar5,uVar6,CONCAT11(extraout_AH,*(undefined1 *)((int)_DAT_2cff_0bf0 + 0x24)));
  }
  FUN_2980_0054(iVar4 + iVar1,uVar6);
  return;
}


