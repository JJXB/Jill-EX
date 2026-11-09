// FUN_26f4_1fb2 @ 26f4:1fb2

void __cdecl16far FUN_26f4_1fb2(uint param_1,int param_2,undefined2 param_3)

{
  int iVar1;
  bool bVar2;
  
  iVar1 = FUN_26f4_1892(0x1b,param_2,param_3);
  if (iVar1 != 0) {
    *(uint *)((int)(undefined2 *)&DAT_2cff_96b9 + iVar1 * 0x1f) = param_1;
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96bf + iVar1 * 0x1f) = 0x10;
    *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + iVar1 * 0x1f) =
         ((uint)(DAT_2cff_96ad < param_2) - (uint)(param_2 < DAT_2cff_96ad)) * 2;
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + iVar1 * 0x1f) = 3;
    FUN_26f4_020c(iVar1);
  }
  DAT_2cff_be38 = DAT_2cff_be38 | 0xc000;
  bVar2 = CARRY2(DAT_2cff_83cc,param_1);
  DAT_2cff_83cc = DAT_2cff_83cc + param_1;
  DAT_2cff_83ce = DAT_2cff_83ce + ((int)param_1 >> 0xf) + (uint)bVar2;
  return;
}


