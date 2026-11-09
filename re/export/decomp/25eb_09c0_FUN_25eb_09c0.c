// FUN_25eb_09c0 @ 25eb:09c0

/* WARNING: Removing unreachable block (ram,0x000268c1) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl16far FUN_25eb_09c0(void)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  undefined2 uVar7;
  
  DAT_2cff_bed0 = 0;
  DAT_2cff_120e = 1;
  FUN_26f4_215f();
  iVar2 = DAT_2cff_96ad / 0x10;
  iVar3 = DAT_2cff_96af / 0x10;
  FUN_22ab_14a9();
  do {
    FUN_1123_00eb(DAT_2cff_b7f2,DAT_2cff_b7f4,0x100,iVar2 * 0x10 + 4,iVar3 * 0x10 + 4);
    do {
      FUN_1415_026f(0);
      if ((DAT_2cff_3c76 != 0) || (DAT_2cff_3c78 != 0)) break;
    } while (DAT_2cff_3c86 == 0);
    ((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar3] =
         ((undefined2 *)&DAT_2cff_43a6)[iVar2 * 0x40 + iVar3] | 0xc000;
    FUN_26f4_12b9(0);
    FUN_26f4_0d52(0);
    FUN_26f4_2506();
    if ((DAT_2cff_3c76 != 0) || (DAT_2cff_3c78 != 0)) {
      iVar2 = iVar2 + ((DAT_2cff_b998 / 2 + -1) * DAT_2cff_3c7a + 1) * DAT_2cff_3c76;
      iVar3 = iVar3 + ((DAT_2cff_b99a / 2 + -1) * DAT_2cff_3c7a + 1) * DAT_2cff_3c78;
      if (iVar2 < 0) {
        iVar2 = 0;
      }
      if (0x7f < iVar2) {
        iVar2 = 0x7f;
      }
      if (iVar3 < 0) {
        iVar3 = 0;
      }
      if (0x3f < iVar3) {
        iVar3 = 0x3f;
      }
      uVar7 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
      if (iVar2 * 0x10 < *(int *)((int)_DAT_2cff_b7f2 + 8)) {
        piVar1 = (int *)((int)_DAT_2cff_b7f2 + 8);
        *piVar1 = *piVar1 + DAT_2cff_b998 * -8;
        uVar7 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
        if (*(int *)((int)_DAT_2cff_b7f2 + 8) < 0) {
          *(undefined2 *)((int)_DAT_2cff_b7f2 + 8) = 0;
        }
        FUN_22ab_14a9();
      }
      uVar7 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
      if (*(int *)((int)_DAT_2cff_b7f2 + 8) + DAT_2cff_b998 * 0x10 + -0x10 <= iVar2 * 0x10) {
        piVar1 = (int *)((int)_DAT_2cff_b7f2 + 8);
        *piVar1 = *piVar1 + DAT_2cff_b998 * 8;
        uVar7 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
        if ((0x80 - DAT_2cff_b998) * 0x10 + 8 <= *(int *)((int)_DAT_2cff_b7f2 + 8)) {
          *(int *)((int)_DAT_2cff_b7f2 + 8) = (0x80 - DAT_2cff_b998) * 0x10 + 8;
        }
        FUN_22ab_14a9();
      }
      uVar7 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
      if (iVar3 * 0x10 < *(int *)((int)_DAT_2cff_b7f2 + 10)) {
        piVar1 = (int *)((int)_DAT_2cff_b7f2 + 10);
        *piVar1 = *piVar1 + DAT_2cff_b99a * -8;
        uVar7 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
        if (*(int *)((int)_DAT_2cff_b7f2 + 10) < 0) {
          *(undefined2 *)((int)_DAT_2cff_b7f2 + 10) = 0;
        }
        FUN_22ab_14a9();
      }
      uVar7 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
      if (*(int *)((int)_DAT_2cff_b7f2 + 10) + (DAT_2cff_b99a + -1) * 0x10 <= iVar3 * 0x10) {
        piVar1 = (int *)((int)_DAT_2cff_b7f2 + 10);
        *piVar1 = *piVar1 + DAT_2cff_b99a * 8;
        uVar7 = (undefined2)((ulong)_DAT_2cff_b7f2 >> 0x10);
        if ((0x41 - DAT_2cff_b99a) * 0x10 <= *(int *)((int)_DAT_2cff_b7f2 + 10)) {
          *(int *)((int)_DAT_2cff_b7f2 + 10) = (0x41 - DAT_2cff_b99a) * 0x10;
        }
        FUN_22ab_14a9();
      }
    }
    iVar4 = FUN_2ce7_000d(DAT_2cff_3c86);
    iVar5 = 0xe;
    piVar6 = (int *)0xc6d;
    do {
      if (iVar4 == *piVar6) {
                    /* WARNING: Could not recover jumptable at 0x00026b19. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (*(code *)piVar6[0xe])();
        return;
      }
      piVar6 = piVar6 + 1;
      iVar5 = iVar5 + -1;
    } while (iVar5 != 0);
    if (DAT_2cff_3c86 == 0x1b) {
      DAT_2cff_3c86 = 0;
      DAT_2cff_120e = 0;
      return;
    }
  } while( true );
}


