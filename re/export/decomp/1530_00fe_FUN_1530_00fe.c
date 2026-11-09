// FUN_1530_00fe @ 1530:00fe

void __cdecl16far FUN_1530_00fe(int param_1,undefined4 param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined2 uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  undefined2 uVar8;
  
  iVar6 = -1;
  if ((DAT_2cff_0aac == 0) &&
     ((DAT_2cff_3e2c == 0 ||
      (((DAT_2cff_3e3c <= param_1 && (DAT_2cff_3e3c != -1)) || (param_1 == -1)))))) {
    if ((-1 < param_1) || (DAT_2cff_3e2c == 0)) {
      DAT_2cff_3e2c = 0;
      DAT_2cff_3e30 = 0;
      DAT_2cff_3e2e = 0;
      DAT_2cff_3e38 = 0;
    }
    DAT_2cff_3e3c = param_1;
    iVar7 = 0;
    do {
      uVar8 = (undefined2)((ulong)param_2 >> 0x10);
      iVar5 = (int)param_2;
      if (*(char *)(iVar5 + iVar7) == -0x10) {
        if (DAT_2cff_3e24 == 0) {
          iVar7 = iVar7 + 2;
        }
        else {
          iVar6 = (int)*(char *)(iVar5 + iVar7 + 1);
          iVar7 = iVar7 + 2;
        }
      }
      iVar1 = (int)*(char *)(iVar5 + iVar7);
      iVar2 = (int)*(char *)(iVar5 + iVar7 + 1);
      iVar7 = iVar7 + 2;
      if (iVar6 == -1) {
        *(undefined2 *)((int)DAT_2cff_0be6 + DAT_2cff_3e2e * 2) = *(undefined2 *)(iVar1 * 2 + 0xab2)
        ;
        *(int *)((int)DAT_2cff_0bea + DAT_2cff_3e2e * 2) = iVar2 * DAT_2cff_3e36;
        DAT_2cff_3e2e = DAT_2cff_3e2e + 1;
        DAT_2cff_3e2c = 1;
      }
      else {
        if (*(int *)(iVar6 * 2 + -0x17ba) < 1) {
          iVar3 = 1;
        }
        else {
          iVar3 = *(int *)(iVar6 * 2 + -0x17ba);
        }
        iVar3 = iVar2 * DAT_2cff_3e36 + iVar3 * -0x80;
        if (iVar3 < 1) {
          if (*(int *)(iVar6 * 2 + -0x17ba) < 1) {
            uVar4 = 1;
          }
          else {
            uVar4 = *(undefined2 *)(iVar6 * 2 + -0x17ba);
          }
          if (*(int *)(iVar6 * 2 + -0x17ba) < 1) {
            iVar3 = 1;
          }
          else {
            iVar3 = *(int *)(iVar6 * 2 + -0x17ba);
          }
          FUN_1530_0003(0xffff,iVar6 * 0x100 + -0x3fba,0x2cff,(iVar2 * DAT_2cff_3e36) / iVar3,uVar4,
                        iVar1);
        }
        else {
          if (*(int *)(iVar6 * 2 + -0x17ba) < 1) {
            uVar4 = 1;
          }
          else {
            uVar4 = *(undefined2 *)(iVar6 * 2 + -0x17ba);
          }
          FUN_1530_0003(0xffff,iVar6 * 0x100 + -0x3fba,0x2cff,0x80,uVar4,iVar1);
          *(undefined2 *)((int)DAT_2cff_0be6 + DAT_2cff_3e2e * 2) = 0xffff;
          *(int *)((int)DAT_2cff_0bea + DAT_2cff_3e2e * 2) = iVar3;
          DAT_2cff_3e2e = DAT_2cff_3e2e + 1;
        }
      }
    } while ((*(char *)(iVar5 + iVar7) != '\0') && (DAT_2cff_3e2e < 0x2000));
  }
  return;
}


