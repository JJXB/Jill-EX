// FUN_11f4_0803 @ 11f4:0803

void __cdecl16far
FUN_11f4_0803(int *param_1,int param_2,int param_3,int param_4,int param_5,char *param_6,int param_7
             )

{
  char *pcVar1;
  char *pcVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  char *pcVar6;
  char *pcVar7;
  undefined2 uVar8;
  char *local_1a;
  undefined4 local_16;
  int local_12;
  int local_10;
  int local_8;
  int local_6;
  int local_4;
  
  piVar5 = (int *)param_1;
  uVar8 = (undefined2)((ulong)param_1 >> 0x10);
  if (param_3 < 0) {
    local_10 = piVar5[1];
    local_6 = param_5 + param_3;
    param_6 = (char *)CONCAT22(param_6._2_2_,(char *)param_6 + -param_3 * param_4);
  }
  else {
    local_10 = piVar5[1] + param_3;
    local_6 = param_5;
  }
  if (piVar5[3] <= param_3 + param_5) {
    local_6 = local_6 - ((param_3 + param_5) - piVar5[3]);
  }
  if (0 < local_6) {
    if (param_7 == 3) {
      if (param_2 < 0) {
        local_12 = *param_1;
        local_8 = param_4 + param_2;
        param_6 = (char *)CONCAT22(param_6._2_2_,(char *)param_6 + -param_2);
        local_4 = -param_2;
      }
      else {
        local_12 = *param_1 + param_2;
        local_8 = param_4;
        local_4 = 0;
      }
      if (piVar5[2] <= param_2 + param_4) {
        local_8 = local_8 - ((param_2 + param_4) - piVar5[2]);
        local_4 = local_4 + ((param_2 + param_4) - piVar5[2]);
      }
      if (0 < local_8) {
        FUN_1123_00a5(local_12,local_10,&local_1a);
        pcVar7 = (char *)local_1a;
        pcVar6 = (char *)param_6;
        do {
          for (iVar3 = local_8; iVar3 != 0; iVar3 = iVar3 + -1) {
            pcVar2 = pcVar7;
            pcVar7 = pcVar7 + 1;
            pcVar1 = pcVar6;
            pcVar6 = pcVar6 + 1;
            *pcVar2 = *pcVar1;
          }
          pcVar7 = pcVar7 + (0x140 - local_8);
          pcVar6 = pcVar6 + local_4;
          local_6 = local_6 + -1;
        } while (local_6 != 0);
      }
    }
    else {
      local_12 = *param_1 + param_2;
      FUN_1123_00a5(local_12,local_10,&local_16);
      iVar3 = piVar5[2];
      for (; 0 < local_6; local_6 = local_6 + -1) {
        local_1a = local_16;
        iVar4 = 0;
        do {
          if ((-param_2 <= iVar4) && (iVar4 <= iVar3 + -param_2 + -1)) {
            if (*(char *)((undefined2 *)&DAT_2cff_3232 + CONCAT11((undefined1)param_7,*param_6)) !=
                -1) {
              *local_1a = *(char *)((undefined2 *)&DAT_2cff_3232 +
                                   CONCAT11((undefined1)param_7,*param_6));
            }
          }
          param_6 = (char *)CONCAT22(param_6._2_2_,(char *)param_6 + 1);
          local_1a = (char *)CONCAT22(local_1a._2_2_,(char *)local_1a + 1);
          iVar4 = iVar4 + 1;
        } while (iVar4 < param_4);
        local_16 = (char *)CONCAT22((int)((ulong)local_16 >> 0x10),(char *)local_16 + 0x140);
      }
    }
  }
  return;
}


