// FUN_11f4_09c6 @ 11f4:09c6

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __cdecl16far
FUN_11f4_09c6(undefined4 param_1,uint param_2,uint param_3,int param_4,int param_5,int param_6,
             int param_7)

{
  int *piVar1;
  char *pcVar2;
  char cVar3;
  uint uVar4;
  int iVar5;
  undefined2 in_CX;
  uint uVar6;
  uint unaff_SI;
  uint unaff_DI;
  undefined2 uVar7;
  undefined2 unaff_SS;
  int local_c [4];
  int iStack_4;
  
  uVar7 = (undefined2)((ulong)param_1 >> 0x10);
  if ((int)param_2 < 0) {
    param_2 = 0;
  }
  else if (*(int *)((int)param_1 + 4) < (int)param_2) {
    param_2 = *(uint *)((int)param_1 + 4);
  }
  if (param_4 < 0) {
    param_4 = 0;
  }
  else if (*(int *)((int)param_1 + 4) < param_4) {
    param_4 = *(int *)((int)param_1 + 4);
  }
  if ((int)param_3 < 0) {
    param_3 = 0;
  }
  else if (*(int *)((int)param_1 + 4) < (int)param_3) {
    param_3 = *(uint *)((int)param_1 + 4);
  }
  if (param_5 < 0) {
    param_5 = 0;
  }
  else if (*(int *)((int)param_1 + 4) < param_5) {
    param_5 = *(int *)((int)param_1 + 4);
  }
  if (((int)param_2 < param_4) && (param_2 = param_3, (int)param_3 < param_5)) {
    local_c[1] = 1;
    local_c[0] = 1;
    if (param_7 < 1) {
      if ((param_7 == 0) && (0 < param_6)) {
        local_c[0] = -1;
      }
    }
    else {
      local_c[1] = 0xffff;
    }
    param_2 = DAT_2cff_3a4d & 0xfe;
    if (param_2 < 7) {
      uVar6 = param_2 * 2;
      switch(param_2) {
      case 0:
        *(undefined2 *)(uVar6 + unaff_DI) = 0xe9e8;
        piVar1 = (int *)(uVar6 + unaff_SI + 3);
        *piVar1 = *piVar1 + 1;
        uVar4 = FUN_1000_010d();
        cVar3 = (char)uVar4;
        *(char *)(uVar6 + unaff_SI) = *(char *)(uVar6 + unaff_SI) + cVar3;
        *(char *)(uVar6 + unaff_SI) = *(char *)(uVar6 + unaff_SI) + cVar3;
        *(char *)(uVar6 + unaff_SI) = *(char *)(uVar6 + unaff_SI) + cVar3;
        *(char *)(uVar6 + unaff_SI) = *(char *)(uVar6 + unaff_SI) + cVar3;
        *(char *)(uVar6 + unaff_SI) = *(char *)(uVar6 + unaff_SI) + cVar3;
        s_No_musical_sound_track_2cff_0689[6] =
             s_No_musical_sound_track_2cff_0689[6] + (char)((uint)in_CX >> 8);
        return uVar4;
      case 2:
        uVar6 = FUN_1123_00eb();
        return uVar6;
      case 6:
        *(undefined1 *)((int)register0x00000010 + (int)((undefined1 *)&DAT_2cff_3dde + unaff_DI)) =
             0;
        pcVar2 = (char *)(CONCAT11((char)(uVar6 >> 8),(char)uVar6 + (char)((uint)in_CX >> 8)) +
                          unaff_SI + 0x2c);
        *pcVar2 = *pcVar2 + (char)in_CX;
        FUN_2c66_0006();
        iVar5 = FUN_1415_0528();
        if (iVar5 == 0) {
          uVar6 = 0;
        }
        else {
          DAT_2cff_3dd0 = DAT_2cff_3d9a;
          FUN_2c48_0006();
          FUN_2c66_0006();
          FUN_2c66_0006();
          FUN_2c66_0006();
          FUN_2c66_0006();
          FUN_2c66_0006();
          FUN_2c66_0006();
          FUN_2c66_0006();
          FUN_2c66_0006();
          iVar5 = FUN_1123_0a21();
          if (iVar5 == 0) {
            uVar6 = 0;
          }
          else {
            DAT_2cff_3dce = 0;
            DAT_2cff_3d9a = DAT_2cff_3dd0;
            DAT_2cff_3dd2 = DAT_2cff_3c8c;
            DAT_2cff_3dd4 = DAT_2cff_3c80;
            DAT_2cff_3dd6 = DAT_2cff_3d8e;
            DAT_2cff_3dd8 = DAT_2cff_3d94;
            DAT_2cff_3dda = DAT_2cff_3c82;
            DAT_2cff_3ddc = DAT_2cff_3c84;
            _DAT_2cff_3dde = (uint)DAT_2cff_3a4d;
            DAT_2cff_3f0e = DAT_2cff_3de2;
            DAT_2cff_42f4 = DAT_2cff_3de0;
            uVar6 = 1;
          }
        }
        return uVar6;
      }
      do {
        unaff_DI = local_c[uVar6 & 3] << ((byte)iStack_4 & 0x1f) | unaff_DI;
        iStack_4 = iStack_4 + 2;
        while (7 < iStack_4) {
          uVar6 = unaff_SI * 2;
          ((undefined2 *)&DAT_2cff_3232)[(int)param_1 * 0x100 + unaff_SI] = unaff_DI;
          unaff_SI = unaff_SI + 1;
          if (0xff < unaff_SI) {
            return uVar6;
          }
          unaff_DI = 0;
          iStack_4 = 0;
        }
        uVar6 = unaff_SI >> ((byte)iStack_4 & 0x1f);
      } while( true );
    }
  }
  return param_2;
}


