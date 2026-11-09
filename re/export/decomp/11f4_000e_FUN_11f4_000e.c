// FUN_11f4_000e @ 11f4:000e

void __cdecl16far
FUN_11f4_000e(int *param_1,uint param_2,int param_3,int param_4,int param_5,undefined1 *param_6,
             int param_7)

{
  undefined1 *puVar1;
  uint *puVar2;
  undefined1 uVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  byte bVar10;
  uint uVar9;
  int *piVar11;
  int iVar12;
  int iVar13;
  undefined1 *puVar14;
  uint *puVar15;
  undefined2 uVar16;
  uint *local_1e;
  uint *local_1a;
  uint local_18;
  char local_15;
  int local_14;
  uint local_12;
  int local_c;
  int local_a;
  uint local_8;
  int local_4;
  
  piVar11 = (int *)param_1;
  uVar16 = (undefined2)((ulong)param_1 >> 0x10);
  if (param_3 < 0) {
    local_12 = piVar11[1];
    local_a = param_5 + param_3;
    param_6 = (undefined1 *)CONCAT22(param_6._2_2_,(undefined1 *)param_6 + -param_3 * param_4);
  }
  else {
    local_12 = piVar11[1] + param_3;
    local_a = param_5;
  }
  if (piVar11[3] <= param_3 + param_5) {
    local_a = local_a - ((param_3 + param_5) - piVar11[3]);
  }
  if (0 < local_a) {
    if ((local_12 & 1) == 0) {
      local_8 = 0x2000;
    }
    else {
      local_8 = 0xe050;
    }
    if ((param_7 == 3) && ((param_2 & 3) == 0)) {
      iVar4 = (int)param_2 >> 2;
      if ((int)param_2 < 0) {
        local_14 = *param_1;
        local_c = iVar4 + param_4;
        param_6 = (undefined1 *)CONCAT22(param_6._2_2_,(undefined1 *)param_6 + -iVar4);
        local_4 = (int)-param_2 >> 2;
      }
      else {
        local_14 = *param_1 + param_2;
        local_c = param_4;
        local_4 = 0;
      }
      if (piVar11[2] <= (int)(param_4 * 4 + param_2)) {
        local_c = local_c - ((iVar4 + param_4) - (piVar11[2] >> 2));
        local_4 = local_4 + ((iVar4 + param_4) - (piVar11[2] >> 2));
      }
      if (0 < local_c) {
        FUN_1123_0002(local_14,local_12,&local_1e);
        puVar15 = (uint *)local_1e;
        puVar14 = (undefined1 *)param_6;
        do {
          for (iVar4 = local_c; iVar4 != 0; iVar4 = iVar4 + -1) {
            puVar2 = puVar15;
            puVar15 = (uint *)((int)puVar15 + 1);
            puVar1 = puVar14;
            puVar14 = puVar14 + 1;
            *(undefined1 *)puVar2 = *puVar1;
          }
          puVar15 = (uint *)((int)puVar15 + (local_8 - local_c));
          local_8 = local_8 ^ 0xc050;
          puVar14 = puVar14 + local_4;
          local_a = local_a + -1;
        } while (local_a != 0);
      }
    }
    else {
      local_14 = *param_1 + param_2;
      FUN_1123_0002(local_14,local_12,&local_1a);
      iVar4 = -((int)(param_2 - 3) / 4 + 1);
      iVar8 = (piVar11[2] / 4 - (int)param_2 / 4) + -2;
      if ((iVar4 + 1 < 1) && (-2 < iVar8)) {
        uVar5 = 0xff00;
      }
      else {
        uVar5 = 0;
      }
      local_1e = (uint *)((ulong)local_18 << 0x10);
      for (; 0 < local_a; local_a = local_a + -1) {
        local_1e = (uint *)CONCAT22(local_1e._2_2_,local_1a);
        iVar12 = 0;
        uVar9 = uVar5;
        do {
          bVar10 = (byte)(uVar9 >> 8);
          uVar9 = (uint)bVar10;
          if ((iVar4 <= iVar12) && (iVar12 <= iVar8)) {
            uVar9 = CONCAT11(0xff,bVar10);
          }
          uVar3 = *param_6;
          param_6 = (undefined1 *)CONCAT22(param_6._2_2_,(undefined1 *)param_6 + 1);
          iVar13 = (((undefined2 *)&DAT_2cff_3232)[CONCAT11((undefined1)param_7,uVar3)] & 0xff) <<
                   (8U - local_15 & 0x1f);
          iVar6 = ((uint)((undefined2 *)&DAT_2cff_3232)[CONCAT11((undefined1)param_7,uVar3)] >> 8)
                  << (8U - local_15 & 0x1f);
          uVar7 = CONCAT11((char)iVar6,(char)((uint)iVar6 >> 8)) & uVar9;
          *local_1e = CONCAT11((char)iVar13,(char)((uint)iVar13 >> 8)) & uVar7 |
                      *local_1e & (uVar7 ^ 0xffff);
          local_1e = (uint *)CONCAT22(local_1e._2_2_,(uint *)((int)(uint *)local_1e + 1));
          iVar12 = iVar12 + 1;
        } while (iVar12 < param_4);
        local_1a = (uint *)((int)local_1a + local_8);
        local_8 = local_8 ^ 0xc050;
      }
    }
  }
  return;
}


