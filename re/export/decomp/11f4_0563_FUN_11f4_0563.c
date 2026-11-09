// FUN_11f4_0563 @ 11f4:0563

uint __cdecl16far
FUN_11f4_0563(int *param_1,uint param_2,int param_3,int param_4,int param_5,char *param_6,
             int param_7)

{
  char *pcVar1;
  char *pcVar2;
  uint uVar3;
  char cVar4;
  int iVar5;
  int *piVar6;
  int iVar7;
  char *pcVar8;
  undefined2 uVar9;
  bool bVar10;
  char *local_22;
  undefined4 local_1e;
  int local_1a;
  int local_18;
  int local_16;
  int local_14;
  int local_12;
  int local_10;
  int local_e;
  uint local_c;
  int local_a;
  uint local_8;
  byte local_5;
  byte local_4;
  byte local_3;
  
  local_e = 0;
  local_c = 0;
  local_a = 0;
  local_8 = 0;
  piVar6 = (int *)param_1;
  uVar9 = (undefined2)((ulong)param_1 >> 0x10);
  if (param_3 < 0) {
    local_18 = piVar6[1];
    local_10 = param_5 + param_3;
    local_e = -param_3 * param_4;
  }
  else {
    local_18 = piVar6[1] + param_3;
    local_10 = param_5;
  }
  uVar3 = piVar6[3];
  if ((int)uVar3 <= param_3 + param_5) {
    local_10 = local_10 - ((param_3 + param_5) - piVar6[3]);
    uVar3 = ((param_3 + param_5) - piVar6[3]) * param_4;
    local_c = uVar3;
  }
  if (0 < local_10) {
    if ((param_7 == 3) && ((param_2 & 3) == 0)) {
      param_4 = param_4 >> 2;
      local_e = local_e >> 2;
      local_c = (int)local_c >> 2;
      iVar7 = (int)param_2 >> 2;
      if ((int)param_2 < 0) {
        local_1a = *param_1;
        local_12 = iVar7 + param_4;
        local_a = (int)-param_2 >> 2;
      }
      else {
        local_1a = *param_1 + param_2;
        local_12 = param_4;
      }
      uVar3 = param_4 * 4 + param_2;
      if (piVar6[2] <= (int)uVar3) {
        local_12 = local_12 - ((iVar7 + param_4) - (piVar6[2] >> 2));
        uVar3 = (iVar7 + param_4) - (piVar6[2] >> 2);
        local_8 = uVar3;
      }
      if (0 < local_12) {
        FUN_1123_00a5(local_1a,local_18,&local_22);
        FUN_2ca4_000c(0x3ce,0x4005);
        FUN_2ca4_000c(0x3ce,1);
        FUN_2ca4_000c(0x3ce,3);
        FUN_2ca4_000c(0x3ce,0xff08);
        local_3 = 8;
        do {
          uVar3 = CONCAT11(local_3,2);
          out(0x3c4,uVar3);
          pcVar8 = (char *)local_22;
          param_6._0_2_ = (char *)param_6 + local_e;
          iVar7 = local_10;
          do {
            param_6._0_2_ = (char *)param_6 + local_a;
            for (iVar5 = local_12; iVar5 != 0; iVar5 = iVar5 + -1) {
              pcVar2 = pcVar8;
              pcVar8 = pcVar8 + 1;
              pcVar1 = (char *)param_6;
              param_6._0_2_ = (char *)param_6 + 1;
              *pcVar2 = *pcVar1;
            }
            pcVar8 = pcVar8 + (0x50 - local_12);
            param_6._0_2_ = (char *)param_6 + local_8;
            iVar7 = iVar7 + -1;
          } while (iVar7 != 0);
          param_6._0_2_ = (char *)param_6 + local_c;
          bVar10 = (bool)(local_3 & 1);
          local_3 = local_3 >> 1 | local_3 << 7;
        } while (!bVar10);
        out(0,uVar3);
      }
    }
    else {
      local_1a = *param_1 + param_2;
      param_6 = (char *)CONCAT22(param_6._2_2_,(char *)param_6 + local_e);
      FUN_1123_00a5(local_1a,local_18,&local_1e);
      local_16 = -param_2;
      local_14 = piVar6[2] + local_16 + -1;
      uVar3 = FUN_2ca4_000c(0x3ce,0x4005);
      for (; 0 < local_10; local_10 = local_10 + -1) {
        local_22 = local_1e;
        local_4 = '\x11' << (local_5 & 0x1f);
        uVar3 = (uint)local_4;
        iVar7 = 0;
        do {
          if ((local_16 <= iVar7) && (iVar7 <= local_14)) {
            cVar4 = (char)((undefined2 *)&DAT_2cff_3232)
                          [CONCAT11((undefined1)param_7,(char)*(undefined2 *)param_6)];
            if (cVar4 != -1) {
              uVar3 = CONCAT11(local_4,2);
              out(0x3c4,uVar3);
              *local_22 = cVar4;
            }
          }
          bVar10 = (char)local_4 < '\0';
          local_4 = local_4 << 1 | bVar10;
          if (bVar10) {
            local_22 = (char *)CONCAT22(local_22._2_2_,(char *)local_22 + 1);
          }
          param_6 = (char *)CONCAT22(param_6._2_2_,(char *)param_6 + 1);
          iVar7 = iVar7 + 1;
        } while (iVar7 < param_4);
        local_1e = (char *)CONCAT22((int)((ulong)local_1e >> 0x10),(char *)local_1e + 0x50);
      }
    }
  }
  return uVar3;
}


