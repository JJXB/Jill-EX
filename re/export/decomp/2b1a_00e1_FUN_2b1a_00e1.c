// FUN_2b1a_00e1 @ 2b1a:00e1

undefined2 __cdecl16far FUN_2b1a_00e1(void)

{
  undefined2 uVar1;
  int iVar2;
  undefined2 uVar3;
  undefined2 uVar4;
  undefined1 uVar5;
  undefined1 uVar6;
  undefined1 uStack0000;
  char local_6 [4];
  
  uVar5 = local_6 + 4 < (char *)s_Turbo_C___Copyright__c__1988_Bor_2cff_0004 + 4;
  uVar6 = &uStack0000 == (char *)s_Turbo_C___Copyright__c__1988_Bor_2cff_0004 + 6;
  uVar3 = DAT_2cff_008b._2_2_;
  uVar1 = FUN_1000_0314();
  FUN_1000_036f(uVar1,uVar3);
  if ((!(bool)uVar5) && (FUN_1000_036f(), (bool)uVar5 || (bool)uVar6)) {
    uVar4 = (undefined2)DAT_2cff_008b;
    iVar2 = FUN_2b1a_000a(uVar1,uVar3);
    if (iVar2 == 0) {
      return 0xffff;
    }
    return uVar4;
  }
  return 0xffff;
}


