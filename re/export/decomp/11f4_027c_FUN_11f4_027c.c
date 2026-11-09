// FUN_11f4_027c @ 11f4:027c

void __cdecl16far
FUN_11f4_027c(int *param_1,uint param_2,int param_3,int param_4,int param_5,uint *param_6,
             int param_7)

{
  byte *pbVar1;
  byte bVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  byte *pbVar6;
  byte *pbVar7;
  byte *pbVar8;
  undefined2 uVar9;
  bool bVar10;
  byte *local_22;
  undefined4 local_1e;
  int local_1a;
  int local_18;
  int local_16;
  int local_14;
  int local_12;
  int local_10;
  int local_e;
  int local_c;
  int local_a;
  int local_8;
  byte local_6;
  byte local_5;
  byte local_4;
  byte local_3;
  
  local_e = 0;
  local_c = 0;
  local_a = 0;
  local_8 = 0;
  piVar4 = (int *)param_1;
  uVar9 = (undefined2)((ulong)param_1 >> 0x10);
  if (param_3 < 0) {
    local_18 = piVar4[1];
    local_10 = param_5 + param_3;
    local_e = -param_3 * param_4;
  }
  else {
    local_18 = piVar4[1] + param_3;
    local_10 = param_5;
  }
  if (piVar4[3] <= param_3 + param_5) {
    local_10 = local_10 - ((param_3 + param_5) - piVar4[3]);
    local_c = ((param_3 + param_5) - piVar4[3]) * param_4;
  }
  if (0 < local_10) {
    if ((param_7 == 3) && ((param_2 & 7) == 0)) {
      param_4 = param_4 >> 3;
      local_e = local_e >> 3;
      local_c = local_c >> 3;
      iVar5 = (int)param_2 >> 3;
      if ((int)param_2 < 0) {
        local_1a = *param_1;
        local_12 = iVar5 + param_4;
        local_a = (int)-param_2 >> 3;
      }
      else {
        local_1a = *param_1 + param_2;
        local_12 = param_4;
      }
      if (piVar4[2] <= (int)(param_4 * 8 + param_2)) {
        local_12 = local_12 - ((iVar5 + param_4) - (piVar4[2] >> 3));
        local_8 = (iVar5 + param_4) - (piVar4[2] >> 3);
      }
      if (0 < local_12) {
        FUN_1123_005d(local_1a,local_18,&local_22);
        local_3 = 8;
        out(0x3ce,0x805);
        out(0x3ce,7);
        out(0x3ce,1);
        out(0x3ce,3);
        out(0x3ce,0xff08);
        do {
          out(0x3c4,CONCAT11(local_3,2));
          pbVar7 = (byte *)local_22;
          pbVar6 = (byte *)((int)(uint *)param_6 + local_e);
          iVar5 = local_10;
          do {
            pbVar6 = pbVar6 + local_a;
            iVar3 = local_12;
            do {
              pbVar8 = pbVar7;
              pbVar1 = pbVar6;
              pbVar6 = pbVar6 + 1;
              *pbVar8 = *pbVar8 & *pbVar1;
              iVar3 = iVar3 + -1;
              pbVar7 = pbVar8 + 1;
            } while (iVar3 != 0);
            pbVar7 = pbVar8 + (0x29 - local_12);
            pbVar6 = pbVar6 + local_8;
            iVar5 = iVar5 + -1;
          } while (iVar5 != 0);
          param_6._0_2_ = (uint *)(pbVar6 + local_c);
          bVar10 = (bool)(local_3 & 1);
          local_3 = local_3 >> 1 | local_3 << 7;
        } while (!bVar10);
      }
    }
    else {
      local_1a = *param_1 + param_2;
      param_6 = (uint *)CONCAT22(param_6._2_2_,(uint *)((int)(uint *)param_6 + (local_e >> 1)));
      FUN_1123_005d(local_1a,local_18,&local_1e);
      local_16 = -param_2;
      local_14 = piVar4[2] + local_16 + -1;
      FUN_2ca4_000c(0x3c4,0xf02);
      FUN_2ca4_000c(0x3ce,0xa05);
      FUN_2ca4_000c(0x3ce,0xf01);
      FUN_2ca4_000c(0x3ce,3);
      FUN_2ca4_000c(0x3ce,7);
      for (; 0 < local_10; local_10 = local_10 + -1) {
        local_22 = local_1e;
        local_6 = 0;
        local_4 = (byte)(0x80 >> (local_5 & 0x1f));
        iVar5 = 0;
        do {
          if ((local_16 <= iVar5) && (iVar5 <= local_14)) {
            bVar2 = (byte)((undefined2 *)&DAT_2cff_3232)
                          [CONCAT11((undefined1)param_7,(char)(*param_6 >> local_6)) & 0xff0f];
            if (bVar2 != 0x10) {
              out(0x3ce,CONCAT11(local_4,8));
              *local_22 = *local_22 & bVar2;
            }
          }
          bVar10 = (bool)(local_4 & 1);
          local_4 = local_4 >> 1 | local_4 << 7;
          if (bVar10) {
            local_22 = (byte *)CONCAT22(local_22._2_2_,(byte *)local_22 + 1);
          }
          local_6 = local_6 ^ 4;
          if (local_6 == 0) {
            param_6 = (uint *)CONCAT22(param_6._2_2_,(uint *)((int)(uint *)param_6 + 1));
          }
          iVar5 = iVar5 + 1;
        } while (iVar5 < param_4);
        local_1e = (byte *)CONCAT22((int)((ulong)local_1e >> 0x10),(byte *)local_1e + 0x28);
      }
    }
  }
  return;
}


