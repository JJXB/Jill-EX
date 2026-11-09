// FUN_1123_02d3 @ 1123:02d3

void __cdecl16far FUN_1123_02d3(int *param_1,int param_2,int param_3,uint param_4)

{
  int *piVar1;
  undefined2 uVar2;
  
  if ((-1 < param_2) && (-1 < param_3)) {
    uVar2 = (undefined2)((ulong)param_1 >> 0x10);
    piVar1 = (int *)param_1;
    if ((param_2 < piVar1[2]) && (param_3 < piVar1[3])) {
      if ((DAT_2cff_3a4d & 0xfe) == 0) {
        FUN_12fd_007d(*param_1 + param_2,piVar1[1] + param_3,param_4 & 3);
      }
      else if ((DAT_2cff_3a4d & 0xfe) == 2) {
        FUN_12fd_009c(*param_1 + param_2,piVar1[1] + param_3,param_4 & 0xf);
      }
      else if ((DAT_2cff_3a4d & 0xfe) == 4) {
        FUN_12fd_00e7(*param_1 + param_2,piVar1[1] + param_3,param_4);
      }
    }
  }
  return;
}


