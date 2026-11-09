// FUN_2ae9_020b @ 2ae9:020b

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint * __cdecl16far FUN_2ae9_020b(uint param_1,int param_2)

{
  uint *puVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  undefined2 uVar5;
  bool bVar6;
  undefined1 uVar7;
  undefined4 local_6;
  
  if (param_1 == 0 && param_2 == 0) {
    puVar1 = (uint *)0x0;
  }
  else {
    uVar4 = param_2 + (uint)(0xffe8 < param_1);
    uVar2 = param_1 + 0x17 & 0xfff0;
    bVar6 = true;
    FUN_1000_036f();
    if (bVar6) {
      puVar1 = (uint *)FUN_2ae9_01a5(uVar2,uVar4);
    }
    else {
      local_6 = (uint *)CONCAT22(_DAT_2cff_284e,DAT_2cff_284c);
      uVar7 = 1;
      FUN_1000_036f();
      while (!(bool)uVar7) {
        uVar5 = (undefined2)((ulong)local_6 >> 0x10);
        puVar1 = (uint *)local_6;
        uVar3 = uVar4 + (0xffcf < uVar2);
        if ((uVar3 <= puVar1[1]) && ((puVar1[1] != uVar3 || (uVar2 + 0x30 <= *local_6)))) {
          puVar1 = (uint *)FUN_2ae9_0087(puVar1,uVar5,uVar2,uVar4);
          return puVar1;
        }
        uVar7 = puVar1[1] == uVar4;
        if ((uVar4 <= puVar1[1]) &&
           ((!(bool)uVar7 || (uVar7 = *local_6 == uVar2, uVar2 <= *local_6)))) {
          FUN_2ae9_001f(puVar1,uVar5);
          uVar2 = *local_6;
          *local_6 = *local_6 + 1;
          puVar1[1] = puVar1[1] + (uint)(0xfffe < uVar2);
          return puVar1 + 4;
        }
        local_6 = *(uint **)(puVar1 + 6);
        FUN_1000_036f();
      }
      puVar1 = (uint *)FUN_2ae9_0137(uVar2,uVar4);
    }
  }
  return puVar1;
}


