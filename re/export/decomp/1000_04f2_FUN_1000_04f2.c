// FUN_1000_04f2 @ 1000:04f2

/* WARNING: Removing unreachable block (ram,0x0001058f) */
/* WARNING: Removing unreachable block (ram,0x0001056e) */

undefined4 __stdcall16far FUN_1000_04f2(int param_1,uint param_2,uint param_3,uint param_4)

{
  ulong uVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  bool bVar5;
  bool bVar6;
  
  if ((param_4 == 0) && ((param_2 == 0 || (param_3 == 0)))) {
    param_1 = (int)(CONCAT22(param_2,param_1) / (ulong)param_3);
    param_2 = 0;
  }
  else {
    bVar6 = (int)param_2 < 0;
    if (bVar6) {
      bVar5 = param_1 != 0;
      param_1 = -param_1;
      param_2 = -(uint)bVar5 - param_2;
    }
    if ((int)param_4 < 0) {
      bVar5 = param_3 != 0;
      param_3 = -param_3;
      param_4 = -(uint)bVar5 - param_4;
      bVar6 = !bVar6;
    }
    iVar2 = 0x20;
    uVar4 = 0;
    uVar3 = 0;
    do {
      bVar5 = param_1 < 0;
      param_1 = param_1 * 2;
      uVar1 = (ulong)CONCAT12(bVar5,param_2) << 1;
      param_2 = (uint)uVar1 | (uint)bVar5;
      bVar5 = (uVar1 & 0x10000) != 0;
      uVar1 = (ulong)CONCAT12(bVar5,uVar3) << 1;
      uVar3 = (uint)uVar1 | (uint)bVar5;
      uVar4 = uVar4 << 1 | (uint)((uVar1 & 0x10000) != 0);
      if ((param_4 <= uVar4) && ((param_4 < uVar4 || (param_3 <= uVar3)))) {
        bVar5 = uVar3 < param_3;
        uVar3 = uVar3 - param_3;
        uVar4 = (uVar4 - param_4) - (uint)bVar5;
        param_1 = param_1 + 1;
      }
      iVar2 = iVar2 + -1;
    } while (iVar2 != 0);
    if (bVar6) {
      bVar6 = param_1 != 0;
      param_1 = -param_1;
      param_2 = -(uint)bVar6 - param_2;
    }
  }
  return CONCAT22(param_2,param_1);
}


