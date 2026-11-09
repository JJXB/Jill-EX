// FUN_11f4_0e77 @ 11f4:0e77

uint __cdecl16far FUN_11f4_0e77(void)

{
  int *piVar1;
  uint uVar2;
  int iVar3;
  int unaff_SI;
  int iVar4;
  
  uVar2 = DAT_2cff_3a4d & 0xfe;
  switch(uVar2) {
  case 0:
  case 6:
    return DAT_2cff_3a4d & 0xe;
  case 2:
    break;
  case 4:
    uVar2 = FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4);
    return uVar2;
  default:
    return uVar2;
  }
  if ((int)((undefined2 *)&DAT_2cff_96b3)[uVar2] < -0xc) {
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) = 0xfff4;
  }
  iVar3 = FUN_26f4_27dd();
  piVar1 = (int *)((int)(undefined2 *)&DAT_2cff_96bb + unaff_SI * 0x1f);
  *piVar1 = *piVar1 + iVar3 * 10;
  iVar3 = FUN_26f4_1d73();
  if (iVar3 == 0) {
    *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + unaff_SI * 0x1f) =
         -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + unaff_SI * 0x1f);
  }
  iVar4 = ((*(int *)((int)(undefined2 *)&DAT_2cff_96af + unaff_SI * 0x1f) +
            *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) & 0xfff0U) + 0x10) -
          DAT_2cff_b7a4;
  iVar3 = FUN_26f4_1d73();
  if ((iVar3 == 0) &&
     ((FUN_1581_0178(1,0x23), *(int *)((int)(undefined2 *)&DAT_2cff_96af + unaff_SI * 0x1f) == iVar4
      || (iVar3 = FUN_26f4_1d73(), iVar3 == 0)))) {
    *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f) =
         -*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + unaff_SI * 0x1f);
  }
  iVar3 = FUN_26f4_1d73();
  if (iVar3 == 0) {
    FUN_26f4_1863();
  }
  return 1;
}


