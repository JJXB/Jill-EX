// FUN_1069_0134 @ 1069:0134

void __cdecl16far
FUN_1069_0134(undefined4 param_1,undefined4 param_2,int param_3,int param_4,int param_5)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  
  iVar3 = param_3 / 2;
  iVar4 = param_4 / 2;
  iVar1 = *(int *)((undefined1 *)&DAT_2cff_2a5c + param_5 * 2);
  uVar2 = *(uint *)(param_5 * 2 + 0x2c5c);
  FUN_2ba4_0007((int)param_1,param_1._2_2_,param_3 * param_4,uVar2 & 0xff00);
  for (iVar7 = 0; iVar7 < param_3; iVar7 = iVar7 + 1) {
    for (iVar8 = 0; iVar8 < param_4; iVar8 = iVar8 + 1) {
      iVar5 = (int)((iVar7 - iVar3) * iVar1 + (iVar8 - iVar4) * uVar2) / 0x100 + iVar3;
      iVar6 = (int)((iVar8 - iVar4) * iVar1 - (iVar7 - iVar3) * uVar2) / 0x100 + iVar4;
      if ((((-1 < iVar5) && (iVar5 < param_3)) && (-1 < iVar6)) && (iVar6 < param_4)) {
        *(undefined1 *)((int)param_1 + iVar7 + iVar8 * param_3) =
             *(undefined1 *)((int)param_2 + iVar5 + iVar6 * param_3);
      }
    }
  }
  return;
}


