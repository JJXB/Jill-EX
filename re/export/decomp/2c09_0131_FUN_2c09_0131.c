// FUN_2c09_0131 @ 2c09:0131

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_2c09_0131(void)

{
  uint *puVar1;
  undefined2 uVar2;
  uint *puVar3;
  undefined2 uVar4;
  bool bVar5;
  undefined1 uVar6;
  undefined1 uStack0000;
  
  bVar5 = &uStack0000 == (char *)s_Turbo_C___Copyright__c__1988_Bor_2cff_0004 + 2;
  FUN_1000_036f();
  if (bVar5) {
    FUN_2b1a_009a(DAT_2cff_2844,DAT_2cff_2846);
    _DAT_2cff_2848 = (uint *)0x0;
    DAT_2cff_2846 = 0;
    DAT_2cff_2844 = 0;
  }
  else {
    uVar4 = (undefined2)((ulong)_DAT_2cff_2848 >> 0x10);
    puVar1 = (uint *)*(undefined4 *)((int)_DAT_2cff_2848 + 4);
    puVar3 = (uint *)puVar1;
    uVar2 = (undefined2)((ulong)puVar1 >> 0x10);
    uVar6 = (*puVar1 & 1) == 0;
    if ((bool)uVar6) {
      FUN_2ae9_001f(puVar3,uVar2);
      FUN_1000_036f();
      if ((bool)uVar6) {
        _DAT_2cff_2848 = (uint *)0x0;
        DAT_2cff_2846 = 0;
        DAT_2cff_2844 = 0;
      }
      else {
        _DAT_2cff_2848 = *(uint **)(puVar3 + 2);
      }
      FUN_2b1a_009a(puVar3,uVar2);
    }
    else {
      FUN_2b1a_009a((int)_DAT_2cff_2848,uVar4);
      _DAT_2cff_2848 = puVar1;
    }
  }
  return;
}


