// FUN_1353_0b64 @ 1353:0b64

void __cdecl16far FUN_1353_0b64(undefined4 param_1,undefined2 param_2,uint param_3)

{
  int iVar1;
  uint uVar2;
  undefined2 uVar3;
  uint local_4;
  
  iVar1 = FUN_2c68_000a(param_2);
  uVar2 = -iVar1;
  if ((DAT_2cff_3a4d & 0xfe) != 0) {
    if ((DAT_2cff_3a4d & 0xfe) == 2) {
      if ((int)uVar2 < 0) {
        uVar2 = FUN_2c68_000a(uVar2);
        uVar2 = (uVar2 & 7) + 8;
        local_4 = uVar2;
      }
      else {
        uVar2 = (uVar2 & 7) + 8;
        local_4 = uVar2 & 7;
      }
      if (param_3 != 0xffff) {
        param_3 = param_3 & 0xf;
      }
    }
    else if ((DAT_2cff_3a4d & 0xfe) == 4) {
      if ((int)uVar2 < 0) {
        uVar2 = FUN_2c68_000a(uVar2);
        uVar2 = (uVar2 & 7) + 8;
        local_4 = uVar2;
      }
      else {
        uVar2 = (uVar2 & 7) + 8;
        local_4 = uVar2 & 7;
      }
    }
  }
  FUN_1123_0541(uVar2,local_4,param_3);
  uVar3 = (undefined2)((ulong)param_1 >> 0x10);
  *(uint *)((int)param_1 + 0xc) = uVar2;
  *(uint *)((int)param_1 + 0xe) = param_3;
  DAT_2cff_3c6a = uVar2;
  DAT_2cff_3c6e = param_3;
  return;
}


