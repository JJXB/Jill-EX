// FUN_1353_023b @ 1353:023b

void __cdecl16far FUN_1353_023b(uint *param_1)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  
  FUN_1353_0b4a((uint *)param_1 + 0xc,param_1._2_2_);
  if ((*param_1 & 2) == 0) {
    FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4302,0,0);
    FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4307,((uint *)param_1)[4] + 8,
                  ((uint *)param_1)[6] + 0x10);
    FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4305,0,((uint *)param_1)[6] + 0x10);
    for (uVar1 = 0; (int)uVar1 < (int)((uint *)param_1)[5]; uVar1 = uVar1 + 1) {
      if (((uint *)param_1)[9] != uVar1 || uVar1 == 0) {
        FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4304,uVar1 * 0x10 + 8,0);
        FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4306,uVar1 * 0x10 + 8,
                      ((uint *)param_1)[6] + 0x10);
      }
    }
    FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4303,((uint *)param_1)[4] + 8,0);
    for (uVar1 = 0; (int)uVar1 < (int)((uint *)param_1)[7]; uVar1 = uVar1 + 1) {
      FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4301,((uint *)param_1)[4] + 8,
                    uVar1 * 0x10 + 0x10);
      if ((uVar1 == 0) || (((uint *)param_1)[0xb] != uVar1)) {
        FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4300,0,uVar1 * 0x10 + 0x10);
      }
    }
    if ((*param_1 & 1) != 0) {
      FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x430e,((uint *)param_1)[4] + 8,0x20);
      FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x430f,((uint *)param_1)[4] + 8,
                    ((uint *)param_1)[6] - 0x10);
    }
    if (0 < (int)((uint *)param_1)[9]) {
      FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x430a,((uint *)param_1)[8] + 8,0);
      for (uVar1 = 0; (int)uVar1 < (int)((uint *)param_1)[7]; uVar1 = uVar1 + 1) {
        if ((uVar1 == 0) || (((uint *)param_1)[0xb] != uVar1)) {
          FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4309,((uint *)param_1)[8] + 8,
                        uVar1 * 0x10 + 0x10);
        }
      }
      FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4308,((uint *)param_1)[8] + 8,
                    ((uint *)param_1)[6] + 0x10);
      if (0 < (int)((uint *)param_1)[10]) {
        FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x430d,0,((uint *)param_1)[10] + 0x10);
        for (iVar2 = 0; iVar2 < (int)((uint *)param_1)[9]; iVar2 = iVar2 + 1) {
          FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x430c,iVar2 * 0x10 + 8,
                        ((uint *)param_1)[10] + 0x10);
        }
        FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x430b,((uint *)param_1)[8] + 8,
                      ((uint *)param_1)[10] + 0x10);
      }
    }
  }
  else {
    FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4701,0,0);
    FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4703,((uint *)param_1)[4] + 8,0);
    FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4706,0,((uint *)param_1)[6] + 8);
    FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4708,((uint *)param_1)[4] + 8,
                  ((uint *)param_1)[6] + 8);
    for (iVar2 = 1; iVar2 < (int)(((uint *)param_1)[5] * 2 + 1); iVar2 = iVar2 + 1) {
      FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4702,iVar2 << 3,0);
      FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4707,iVar2 << 3,((uint *)param_1)[6] + 8);
    }
    for (iVar2 = 1; iVar2 < (int)(((uint *)param_1)[7] * 2 + 1); iVar2 = iVar2 + 1) {
      FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4704,0,iVar2 << 3);
      FUN_1123_00eb((uint *)param_1 + 0xc,param_1._2_2_,0x4705,((uint *)param_1)[4] + 8,iVar2 << 3);
    }
    for (iVar2 = 0; iVar2 < (int)(((uint *)param_1)[5] * 2); iVar2 = iVar2 + 1) {
      for (iVar3 = 0; iVar3 < (int)(((uint *)param_1)[7] * 2); iVar3 = iVar3 + 1) {
        FUN_1123_00eb((uint *)param_1 + 0x14,param_1._2_2_,0x4709,iVar2 << 3,iVar3 << 3);
      }
    }
  }
  return;
}


