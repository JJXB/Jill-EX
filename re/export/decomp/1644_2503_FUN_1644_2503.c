// FUN_1644_2503 @ 1644:2503

/* WARNING: Removing unreachable block (ram,0x00018acf) */
/* WARNING: Removing unreachable block (ram,0x00018ad8) */
/* WARNING: Removing unreachable block (ram,0x00018b52) */
/* WARNING: Removing unreachable block (ram,0x00018adf) */
/* WARNING: Removing unreachable block (ram,0x00018b48) */
/* WARNING: Removing unreachable block (ram,0x00018b4e) */
/* WARNING: Removing unreachable block (ram,0x00018ad5) */
/* WARNING: Removing unreachable block (ram,0x00018b58) */
/* WARNING: Removing unreachable block (ram,0x00018ba3) */
/* WARNING: Removing unreachable block (ram,0x00018b5a) */
/* WARNING: Removing unreachable block (ram,0x00018bc1) */

long __cdecl16near FUN_1644_2503(undefined2 param_1,uint param_2,uint param_3,uint param_4)

{
  byte *pbVar1;
  int *piVar2;
  int iVar3;
  undefined2 in_CX;
  uint uVar4;
  undefined2 uVar5;
  char cVar6;
  int iVar7;
  bool bVar8;
  int local_e;
  int local_c;
  int local_a;
  int local_8;
  int local_6 [2];
  
  iVar3 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_2 * 0x1f) / 0x10;
  uVar4 = (uint)((ulong)param_2 * 0x1f >> 0x10);
  if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + (int)((ulong)param_2 * 0x1f)) == 0) {
    local_a = 0x8c;
  }
  else {
    local_a = 0x1b0;
  }
  local_8 = 0;
  if (5 < param_3) {
    return (ulong)uVar4 << 0x10;
  }
  iVar7 = param_3 * 2;
  bVar8 = (int)param_3 < 0 != iVar7 < 0;
  switch(param_3) {
  case 0:
    FUN_22ab_05b4();
    *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c9 + (int)((ulong)param_4 * 0x1f)) = 3;
    return (ulong)param_4 * 0x1f;
  case 1:
    *(uint *)(iVar7 + 0x13) = param_3;
    piVar2 = (int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_2 * 0x1f);
    *piVar2 = *piVar2 + 1;
    if (0x13 < *(int *)((int)(undefined2 *)&DAT_2cff_96b9 + param_2 * 0x1f)) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b9 + param_2 * 0x1f) = 0;
    }
    FUN_26f4_27dd();
    FUN_26f4_1d73();
    uVar5 = (undefined2)((ulong)param_2 * 0x1f >> 0x10);
    iVar3 = FUN_26f4_1d73();
    if (iVar3 == 0) {
      FUN_26f4_1863();
    }
    else {
      uVar5 = (undefined2)((ulong)param_2 * 0x1f >> 0x10);
      iVar3 = (int)((ulong)param_2 * 0x1f);
      piVar2 = (int *)((int)(undefined2 *)&DAT_2cff_96bb + iVar3);
      *piVar2 = *piVar2 + 1;
      if (0x4f < *(int *)((int)(undefined2 *)&DAT_2cff_96bb + iVar3)) {
        FUN_26f4_1863();
      }
    }
    return CONCAT22(uVar5,1);
  case 2:
    FUN_2c66_0006();
    do {
      do {
        iVar3 = FUN_1333_000f();
      } while (iVar3 == 0);
      FUN_1333_0066();
      iVar3 = FUN_2ce7_000d();
    } while (((iVar3 != 0x4b) && (iVar3 != 0x4a)) && (iVar3 != 0x1b));
    FUN_2c66_0006();
    DAT_2cff_3d9a = 0;
    if (iVar3 == 0x4a) {
      DAT_2cff_3d9a = FUN_1415_0185();
    }
    return CONCAT22(uVar4,(uint)(iVar3 != 0x1b));
  case 3:
    break;
  case 4:
switchD_1000_89ba_caseD_4:
    if (*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_3 * 0x1f) < -0xc) {
      *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_2 * 0x1f) = 0xfff4;
    }
    iVar3 = FUN_26f4_27dd();
    piVar2 = (int *)((int)(undefined2 *)&DAT_2cff_96bb + param_2 * 0x1f);
    *piVar2 = *piVar2 + iVar3 * 10;
    iVar3 = FUN_26f4_1d73();
    if (iVar3 == 0) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_2 * 0x1f) =
           -*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_2 * 0x1f);
    }
    iVar7 = ((*(int *)((int)(undefined2 *)&DAT_2cff_96af + param_2 * 0x1f) +
              *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_2 * 0x1f) & 0xfff0U) + 0x10) -
            DAT_2cff_b7a4;
    iVar3 = FUN_26f4_1d73();
    if ((iVar3 == 0) &&
       ((FUN_1581_0178(), *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_2 * 0x1f) == iVar7 ||
        (iVar3 = FUN_26f4_1d73(), iVar3 == 0)))) {
      *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_2 * 0x1f) =
           -*(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_2 * 0x1f);
    }
    uVar5 = (undefined2)((ulong)param_2 * 0x1f >> 0x10);
    iVar3 = FUN_26f4_1d73();
    if (iVar3 == 0) {
      FUN_26f4_1863();
    }
    return CONCAT22(uVar5,1);
  case 5:
    cVar6 = (char)iVar7 + (char)((uint)in_CX >> 8);
    *(char *)(iVar3 + -0x3e) = *(char *)(iVar3 + -0x3e) + cVar6;
    pbVar1 = (byte *)(CONCAT11((char)((uint)iVar7 >> 8),cVar6) + param_2);
    *pbVar1 = *pbVar1 | (byte)param_3;
    uVar5 = 0xff;
    FUN_1000_0414();
    return CONCAT22(uVar5,(uint)(byte)((char)uVar5 + 1));
  }
  do {
    if (bVar8 != iVar7 < 0) {
      local_c = local_e;
      local_a = local_8;
    }
    do {
      iVar3 = iVar3 + 1;
      if (DAT_2cff_becc <= iVar3) {
        if (-1 < local_c) {
          FUN_26f4_263a(local_c,param_2,local_6);
        }
        local_6[1] = 1;
        piVar2 = (int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_2 * 0x1f);
        *piVar2 = *piVar2 + local_6[0];
        if (0xc < *(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_2 * 0x1f)) {
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_2 * 0x1f) = 0xc;
        }
        if (*(int *)((int)(undefined2 *)&DAT_2cff_96b1 + param_2 * 0x1f) < -0xc) {
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b1 + param_2 * 0x1f) = 0xfff4;
        }
        piVar2 = (int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_2 * 0x1f);
        *piVar2 = *piVar2 + 1;
        if (0xc < *(int *)((int)(undefined2 *)&DAT_2cff_96b3 + param_2 * 0x1f)) {
          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b3 + param_2 * 0x1f) = 0xc;
        }
        param_3 = param_2;
        goto switchD_1000_89ba_caseD_4;
      }
      local_e = ((undefined2 *)&DAT_2cff_bcac)[iVar3];
    } while ((((((undefined2 *)&DAT_2cff_be3c)[(char)((undefined1 *)&DAT_2cff_96ac)[local_e * 0x1f]]
               & 0x400) == 0) || (local_e == 0)) || (local_8 = FUN_26f4_2772(), local_a <= local_8))
    ;
    bVar8 = SBORROW2(local_8,0x60);
    iVar7 = local_8 + -0x60;
  } while( true );
}


