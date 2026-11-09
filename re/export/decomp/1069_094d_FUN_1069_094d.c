// FUN_1069_094d @ 1069:094d

void __cdecl16far FUN_1069_094d(void)

{
  undefined2 uVar1;
  int iVar2;
  int in_DX;
  int iVar3;
  undefined2 unaff_SS;
  undefined2 local_506 [256];
  int local_306 [256];
  int local_106 [128];
  int local_6;
  int local_4;
  
  local_6 = FUN_2ae9_000a(0x1000);
  local_4 = in_DX;
  if (local_6 == 0 && in_DX == 0) {
    FUN_22ab_3177(9);
  }
  for (iVar3 = 0; iVar3 < 0x40; iVar3 = iVar3 + 1) {
    local_306[iVar3 * 2 + 1] = 0;
    local_306[iVar3 * 2] = 0;
  }
  uVar1 = FUN_2b4b_000c(0x2f5c,0x2cff,0x8000);
  iVar3 = FUN_2b58_000d(uVar1,local_506);
  if (iVar3 == 0) {
    FUN_22ab_3177(9);
  }
  FUN_2b58_000d(uVar1,local_106);
  for (iVar3 = 0; iVar3 < 0x40; iVar3 = iVar3 + 1) {
    if (((undefined2 *)&DAT_2cff_2fae)[iVar3] == 0) {
      if (((undefined2 *)&DAT_2cff_312e)[iVar3 * 2] != 0 ||
          ((undefined2 *)&DAT_2cff_3130)[iVar3 * 2] != 0) {
        FUN_2c09_0008(((undefined2 *)&DAT_2cff_312e)[iVar3 * 2],
                      ((undefined2 *)&DAT_2cff_3130)[iVar3 * 2]);
        ((undefined2 *)&DAT_2cff_3130)[iVar3 * 2] = 0;
        ((undefined2 *)&DAT_2cff_312e)[iVar3 * 2] = 0;
      }
    }
    else if ((((undefined2 *)&DAT_2cff_312e)[iVar3 * 2] == 0 &&
              ((undefined2 *)&DAT_2cff_3130)[iVar3 * 2] == 0) && (local_106[iVar3] != 0)) {
      FUN_2b82_0004(uVar1,local_506[iVar3 * 2],local_506[iVar3 * 2 + 1],0);
      iVar2 = FUN_2ae9_000a(local_106[iVar3]);
      local_306[iVar3 * 2 + 1] = in_DX;
      local_306[iVar3 * 2] = iVar2;
      if (local_306[iVar3 * 2] == 0 && local_306[iVar3 * 2 + 1] == 0) {
        FUN_22ab_3177(9);
      }
      FUN_2b58_000d(uVar1,local_306[iVar3 * 2],local_306[iVar3 * 2 + 1],local_106[iVar3]);
    }
  }
  FUN_2b53_0006(uVar1);
  for (iVar3 = 0; iVar3 < 0x40; iVar3 = iVar3 + 1) {
    if (local_306[iVar3 * 2] != 0 || local_306[iVar3 * 2 + 1] != 0) {
      FUN_1069_0271(iVar3,local_306[iVar3 * 2],local_306[iVar3 * 2 + 1],local_6,local_4);
      FUN_2c09_0008(local_306[iVar3 * 2],local_306[iVar3 * 2 + 1]);
    }
  }
  FUN_2c09_0008(local_6,local_4);
  return;
}


