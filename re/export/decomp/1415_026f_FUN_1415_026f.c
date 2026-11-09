// FUN_1415_026f @ 1415:026f

void __cdecl16far FUN_1415_026f(int param_1)

{
  int iVar1;
  uint *puVar2;
  int local_8;
  int local_6;
  int local_4;
  
  if (DAT_2cff_3dc8 == 0) {
    DAT_2cff_3c76 = 0;
    DAT_2cff_3c78 = 0;
    DAT_2cff_3c7a = 0;
    DAT_2cff_3c7c = 0;
    DAT_2cff_3c86 = 0;
    iVar1 = FUN_1333_000f();
    if ((iVar1 != 0) && (DAT_2cff_3c86 = FUN_1333_0066(), DAT_2cff_3c86 < 3)) {
      DAT_2cff_3c86 = FUN_1333_0066();
    }
    if (DAT_2cff_3c86 != 0) {
      iVar1 = 8;
      puVar2 = (uint *)0x317;
      do {
        if (DAT_2cff_3c86 == *puVar2) {
                    /* WARNING: Could not recover jumptable at 0x00014463. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          (*(code *)puVar2[8])();
          return;
        }
        puVar2 = puVar2 + 1;
        iVar1 = iVar1 + -1;
      } while (iVar1 != 0);
    }
    FUN_1333_00ad();
    DAT_2cff_3c7a = (uint)DAT_2cff_3c65;
    DAT_2cff_3c7e = (uint)DAT_2cff_3a63;
    if (((DAT_2cff_3c76 == 0) && (DAT_2cff_3c78 == 0)) && (DAT_2cff_3d9a != 0)) {
      FUN_1415_0030(&local_8);
      local_4 = local_6 - DAT_2cff_3c82;
      DAT_2cff_3c76 =
           (uint)(DAT_2cff_3d8e < (local_8 - DAT_2cff_3c80) * 2) -
           (uint)((local_8 - DAT_2cff_3c80) * 2 < DAT_2cff_3c8c);
      DAT_2cff_3c78 = (uint)(DAT_2cff_3c84 < local_4 * 2) - (uint)(local_4 * 2 < DAT_2cff_3d94);
      iVar1 = FUN_1415_0004();
      if (iVar1 != 0) {
        DAT_2cff_3c7a = 1;
      }
      iVar1 = FUN_1415_001a();
      if (iVar1 != 0) {
        DAT_2cff_3c7e = 1;
      }
    }
    if (((DAT_2cff_3c76 == 0) && (DAT_2cff_3c78 == 0)) && (param_1 != 0)) {
      if ((DAT_2cff_3aaf != '\0') || (DAT_2cff_3baf != '\0')) {
        DAT_2cff_3c76 = -1;
      }
      if ((DAT_2cff_3ab1 != '\0') || (DAT_2cff_3bb1 != '\0')) {
        DAT_2cff_3c76 = DAT_2cff_3c76 + 1;
      }
      if ((DAT_2cff_3aac != '\0') || (DAT_2cff_3bac != '\0')) {
        DAT_2cff_3c78 = -1;
      }
      if ((DAT_2cff_3ab4 != '\0') || (DAT_2cff_3bb4 != '\0')) {
        DAT_2cff_3c78 = DAT_2cff_3c78 + 1;
      }
    }
    if (DAT_2cff_3c7a == 0) {
      DAT_2cff_3d9e = 0;
    }
    else {
      DAT_2cff_3c7a = DAT_2cff_3c7a ^ DAT_2cff_3d9e;
    }
    if (DAT_2cff_3c7e == 0) {
      DAT_2cff_3da0 = 0;
    }
    else {
      DAT_2cff_3c7e = DAT_2cff_3c7e ^ DAT_2cff_3da0;
    }
    DAT_2cff_3c88 = DAT_2cff_3c76;
    DAT_2cff_3c8a = DAT_2cff_3c78;
    if (DAT_2cff_3dc2 == 0) {
      if ((((DAT_2cff_120a != 0) && (DAT_2cff_120c != 0)) && (DAT_2cff_3c7a != 0)) &&
         (DAT_2cff_3c7e != 0)) {
        FUN_150f_0008(10);
      }
    }
    else {
      FUN_1415_0731();
    }
  }
  else {
    FUN_1415_093b();
  }
  return;
}


