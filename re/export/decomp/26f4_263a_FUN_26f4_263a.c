// FUN_26f4_263a @ 26f4:263a

void __cdecl16far FUN_26f4_263a(int param_1,int param_2,int *param_3,int *param_4,int param_5)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  
  uVar4 = *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_1 * 0x1f) -
          *(int *)((int)(undefined2 *)&DAT_2cff_96ad + param_2 * 0x1f);
  iVar3 = *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_1 * 0x1f) -
          *(int *)((int)(undefined2 *)&DAT_2cff_96af + param_2 * 0x1f);
  if (uVar4 == 0) {
    if (iVar3 != 0) {
      iVar3 = ((uint)(0 < iVar3) - (uint)(iVar3 < 0)) * param_5;
    }
    iVar1 = 0;
  }
  else if (iVar3 == 0) {
    iVar1 = ((uint)(0 < (int)uVar4) - (uint)((int)uVar4 < 0)) * param_5;
  }
  else {
    iVar1 = iVar3;
    if (iVar3 < 0) {
      iVar1 = -iVar3;
    }
    if (iVar1 < (int)((uVar4 ^ (int)uVar4 >> 0xf) - ((int)uVar4 >> 0xf))) {
      uVar2 = uVar4;
      if ((int)uVar4 < 0) {
        uVar2 = -uVar4;
      }
      iVar3 = (iVar3 * param_5) / (int)uVar2;
      iVar1 = ((uint)(0 < (int)uVar4) - (uint)((int)uVar4 < 0)) * param_5;
    }
    else {
      iVar1 = iVar3;
      if (iVar3 < 0) {
        iVar1 = -iVar3;
      }
      iVar1 = (int)(uVar4 * param_5) / iVar1;
      iVar3 = ((uint)(0 < iVar3) - (uint)(iVar3 < 0)) * param_5;
    }
  }
  *param_3 = iVar1;
  *param_4 = iVar3;
  return;
}


