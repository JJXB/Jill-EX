// FUN_2b31_003f @ 2b31:003f

int __cdecl16far FUN_2b31_003f(undefined2 param_1,undefined2 param_2,uint param_3,uint param_4)

{
  byte bVar1;
  int iVar2;
  uint uVar3;
  bool bVar4;
  
  if ((param_3 & 0xc000) == 0) {
    param_3 = param_3 | DAT_2cff_297c & 0xc000;
  }
  if ((param_3 & 0x100) == 0) {
LAB_2b31_0100:
    bVar4 = false;
  }
  else {
    param_4 = param_4 & DAT_2cff_297e;
    if ((param_4 & 0x180) == 0) {
      FUN_2adf_000c(1);
    }
    iVar2 = FUN_2bb0_0000(param_1,param_2,0);
    if (iVar2 != -1) {
      if ((param_3 & 0x400) != 0) {
        iVar2 = FUN_2adf_000c(0x50);
        return iVar2;
      }
      goto LAB_2b31_0100;
    }
    bVar4 = (param_4 & 0x80) == 0;
    if ((param_3 & 0xf0) == 0) {
      iVar2 = FUN_2b31_000c(bVar4,param_1,param_2);
      if (iVar2 < 0) {
        return iVar2;
      }
      goto LAB_2b31_0181;
    }
    iVar2 = FUN_2b31_000c(0,param_1,param_2);
    if (iVar2 < 0) {
      return iVar2;
    }
    FUN_2b56_0007(iVar2);
  }
  iVar2 = FUN_2b4b_000c(param_1,param_2,param_3);
  if (-1 < iVar2) {
    bVar1 = FUN_2b50_0009(iVar2,0);
    if ((bVar1 & 0x80) == 0) {
      uVar3 = param_3;
      if ((param_3 & 0x200) != 0) {
        FUN_2b31_002b(iVar2);
      }
    }
    else {
      uVar3 = param_3 | 0x2000;
      if ((param_3 & 0x8000) != 0) {
        FUN_2b50_0009(iVar2,1,bVar1 | 0x20,0);
      }
    }
    param_3 = uVar3;
    if ((bVar4) && ((uVar3 & 0xf0) != 0)) {
      FUN_2bb0_0000(param_1,param_2,1,1);
    }
  }
LAB_2b31_0181:
  if (-1 < iVar2) {
    if ((param_3 & 0x300) == 0) {
      uVar3 = 0;
    }
    else {
      uVar3 = 0x1000;
    }
    *(uint *)(iVar2 * 2 + 0x2954) = uVar3 | param_3 & 0xf8ff;
  }
  return iVar2;
}


