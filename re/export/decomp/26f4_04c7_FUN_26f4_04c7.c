// FUN_26f4_04c7 @ 26f4:04c7

void __cdecl16far FUN_26f4_04c7(int param_1)

{
  uint uVar1;
  undefined2 uVar2;
  undefined2 uVar3;
  undefined2 uVar4;
  int iVar5;
  int local_c;
  int local_a;
  
  DAT_2cff_be38 = DAT_2cff_be38 | 0xc000;
  iVar5 = FUN_26f4_037d(DAT_2cff_83a6);
  uVar4 = DAT_2cff_83d4;
  uVar3 = DAT_2cff_83d2;
  uVar2 = DAT_2cff_83a6;
  uVar1 = *(uint *)((int)(undefined2 *)&DAT_2cff_96ad + iVar5 * 0x1f);
  local_c = *(int *)((int)(undefined2 *)&DAT_2cff_96af + iVar5 * 0x1f);
  if (DAT_2cff_96ac != '\x17') {
    local_c = local_c + -0x10;
  }
  if (((0 < iVar5) && (param_1 != 0)) &&
     (*(int *)((int)(undefined2 *)&DAT_2cff_96b9 + iVar5 * 0x1f) == 1)) {
    FUN_22ab_0aa5((undefined1 *)&DAT_2cff_bafc,0x2cff);
    DAT_2cff_83ce = uVar4;
    DAT_2cff_83cc = uVar3;
    DAT_2cff_83a8 = 6;
    DAT_2cff_83a6 = uVar2;
    FUN_26f4_037d(uVar2);
  }
  DAT_2cff_83d4 = DAT_2cff_83ce;
  DAT_2cff_83d2 = DAT_2cff_83cc;
  FUN_26f4_03c4();
  DAT_2cff_96ad = uVar1 & 0xfff8;
  DAT_2cff_96af = local_c;
  FUN_26f4_215f();
  for (iVar5 = 0; iVar5 < 0x80; iVar5 = iVar5 + 1) {
    for (local_a = 0; local_a < 0x40; local_a = local_a + 1) {
      ((undefined2 *)&DAT_2cff_43a6)[iVar5 * 0x40 + local_a] =
           ((undefined2 *)&DAT_2cff_43a6)[iVar5 * 0x40 + local_a] | 0xc000;
    }
  }
  DAT_2cff_96b9 = 4;
  DAT_2cff_96bd = 0;
  return;
}


